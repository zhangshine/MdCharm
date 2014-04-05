#include "qregularexpression.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QMutex>
#include <QtCore/QVector>
#include <QtCore/QStringList>
#include <QtCore/QDebug>
#include <QtCore/QThreadStorage>
#include <QtCore/qglobal.h>

#include <pcre.h>

// after how many usages we optimize the regexp
static const unsigned int qt_qregularexpression_optimize_after_use_count = 10;

/*!
    \internal
*/
static int convertToPcreOptions(QRegularExpression::PatternOptions patternOptions)
{
    int options = 0;

    if (patternOptions & QRegularExpression::CaseInsensitiveOption)
        options |= PCRE_CASELESS;
    if (patternOptions & QRegularExpression::DotMatchesEverythingOption)
        options |= PCRE_DOTALL;
    if (patternOptions & QRegularExpression::MultilineOption)
        options |= PCRE_MULTILINE;
    if (patternOptions & QRegularExpression::ExtendedPatternSyntaxOption)
        options |= PCRE_EXTENDED;
    if (patternOptions & QRegularExpression::InvertedGreedinessOption)
        options |= PCRE_UNGREEDY;
    if (patternOptions & QRegularExpression::DontCaptureOption)
        options |= PCRE_NO_AUTO_CAPTURE;
    if (patternOptions & QRegularExpression::UseUnicodePropertiesOption)
        options |= PCRE_UCP;

    return options;
}

/*!
    \internal
*/
static int convertToPcreOptions(QRegularExpression::MatchOptions matchOptions)
{
    int options = 0;

    if (matchOptions & QRegularExpression::AnchoredMatchOption)
        options |= PCRE_ANCHORED;

    return options;
}

struct QRegularExpressionPrivate : QSharedData
{
    QRegularExpressionPrivate();
    ~QRegularExpressionPrivate();
    QRegularExpressionPrivate(const QRegularExpressionPrivate &other);

    void cleanCompiledPattern();
    void compilePattern();
    void getPatternInfo();
    pcre16_extra *optimizePattern();

    QRegularExpressionMatchPrivate *doMatch(const QString &subject,
                                            int offset,
                                            QRegularExpression::MatchType matchType,
                                            QRegularExpression::MatchOptions matchOptions,
                                            bool checkSubjectString = true,
                                            const QRegularExpressionMatchPrivate *previous = 0) const;

    int captureIndexForName(const QString &name) const;

    QString pattern;
    QRegularExpression::PatternOptions patternOptions;

    // *All* of the following members are set managed while holding this mutex,
    // except for isDirty which is set to true by QRegularExpression setters
    // (right after a detach happened).
    // On the other hand, after the compilation and studying,
    // it's safe to *use* (i.e. read) them from multiple threads at the same time.
    // Therefore, doMatch doesn't need to lock this mutex.
    QMutex mutex;

    // The PCRE pointers are reference-counted by the QRegularExpressionPrivate
    // objects themselves; when the private is copied (i.e. a detach happened)
    // they are set to 0
    pcre16 *compiledPattern;
    pcre16_extra *studyData;
    const char *errorString;
    int errorOffset;
    int capturingCount;
    unsigned int usedCount;
    bool usingCrLfNewlines;
    bool isDirty;
};

struct QRegularExpressionMatchPrivate : QSharedData
{
    QRegularExpressionMatchPrivate(const QRegularExpression &re,
                                   const QString &subject,
                                   QRegularExpression::MatchType matchType,
                                   QRegularExpression::MatchOptions matchOptions,
                                   int capturingCount);

    QRegularExpressionMatch nextMatch() const;

    const QRegularExpression regularExpression;
    const QString subject;
    // the capturedOffsets vector contains pairs of (start, end) positions
    // for each captured substring
    QVector<int> capturedOffsets;

    const QRegularExpression::MatchType matchType;
    const QRegularExpression::MatchOptions matchOptions;

    int capturedCount;

    bool hasMatch;
    bool hasPartialMatch;
    bool isValid;
};

struct QRegularExpressionMatchIteratorPrivate : QSharedData
{
    QRegularExpressionMatchIteratorPrivate(const QRegularExpression &re,
                                           QRegularExpression::MatchType matchType,
                                           QRegularExpression::MatchOptions matchOptions,
                                           const QRegularExpressionMatch &next);

    bool hasNext() const;
    QRegularExpressionMatch next;
    const QRegularExpression regularExpression;
    const QRegularExpression::MatchType matchType;
    const QRegularExpression::MatchOptions matchOptions;
};

/*!
    \internal
*/
QRegularExpression::QRegularExpression(QRegularExpressionPrivate &dd)
    : d(&dd)
{
}

/*!
    \internal
*/
QRegularExpressionPrivate::QRegularExpressionPrivate()
    : pattern(), patternOptions(0),
      mutex(),
      compiledPattern(0), studyData(0),
      errorString(0), errorOffset(-1),
      capturingCount(0),
      usedCount(0),
      usingCrLfNewlines(false),
      isDirty(true)
{
}

/*!
    \internal
*/
QRegularExpressionPrivate::~QRegularExpressionPrivate()
{
    cleanCompiledPattern();
}

/*!
    \internal

    Copies the private, which means copying only the pattern and the pattern
    options. The compiledPattern and the studyData pointers are NOT copied (we
    do not own them any more), and in general all the members set when
    compiling a pattern are set to default values. isDirty is set back to true
    so that the pattern has to be recompiled again.
*/
QRegularExpressionPrivate::QRegularExpressionPrivate(const QRegularExpressionPrivate &other)
    : QSharedData(other),
      pattern(other.pattern), patternOptions(other.patternOptions),
      mutex(),
      compiledPattern(0), studyData(0),
      errorString(0),
      errorOffset(-1), capturingCount(0),
      usedCount(0),
      usingCrLfNewlines(false), isDirty(true)
{
}

/*!
    \internal
*/
void QRegularExpressionPrivate::cleanCompiledPattern()
{
    pcre16_free(compiledPattern);
    pcre16_free_study(studyData);
    usedCount = 0;
    compiledPattern = 0;
    studyData = 0;
    usingCrLfNewlines = false;
    errorOffset = -1;
    capturingCount = 0;
}

/*!
    \internal
*/
void QRegularExpressionPrivate::compilePattern()
{
    QMutexLocker lock(&mutex);

    if (!isDirty)
        return;

    isDirty = false;
    cleanCompiledPattern();

    int options = convertToPcreOptions(patternOptions);
    options |= PCRE_UTF16;

    int errorCode;
    compiledPattern = pcre16_compile2(pattern.utf16(), options,
                                      &errorCode, &errorString, &errorOffset, 0);

    if (!compiledPattern)
        return;

    Q_ASSERT(errorCode == 0);
    Q_ASSERT(studyData == 0); // studying (=>optimizing) is always done later
    errorOffset = -1;

    getPatternInfo();
}

/*!
    \internal
*/
void QRegularExpressionPrivate::getPatternInfo()
{
    Q_ASSERT(compiledPattern);

    pcre16_fullinfo(compiledPattern, 0, PCRE_INFO_CAPTURECOUNT, &capturingCount);

    // detect the settings for the newline
    int patternNewlineSetting;
    pcre16_fullinfo(compiledPattern, studyData, PCRE_INFO_OPTIONS, &patternNewlineSetting);
    patternNewlineSetting &= PCRE_NEWLINE_CR  | PCRE_NEWLINE_LF | PCRE_NEWLINE_CRLF
            | PCRE_NEWLINE_ANY | PCRE_NEWLINE_ANYCRLF;
    if (patternNewlineSetting == 0) {
        // no option was specified in the regexp, grab PCRE build defaults
        int pcreNewlineSetting;
        pcre16_config(PCRE_CONFIG_NEWLINE, &pcreNewlineSetting);
        switch (pcreNewlineSetting) {
        case 13:
            patternNewlineSetting = PCRE_NEWLINE_CR; break;
        case 10:
            patternNewlineSetting = PCRE_NEWLINE_LF; break;
        case 3338: // (13<<8 | 10)
            patternNewlineSetting = PCRE_NEWLINE_CRLF; break;
        case -2:
            patternNewlineSetting = PCRE_NEWLINE_ANYCRLF; break;
        case -1:
            patternNewlineSetting = PCRE_NEWLINE_ANY; break;
        default:
            qWarning("QRegularExpressionPrivate::compilePattern(): "
                     "PCRE_CONFIG_NEWLINE returned an unknown newline");
            break;
        }
    }

    usingCrLfNewlines = (patternNewlineSetting == PCRE_NEWLINE_CRLF) ||
            (patternNewlineSetting == PCRE_NEWLINE_ANY) ||
            (patternNewlineSetting == PCRE_NEWLINE_ANYCRLF);
}


/*!
    \class QPcreJitStackPointer
    \internal

    Simple "smartpointer" wrapper around a pcre_jit_stack, to be used with
    QThreadStorage.
*/
class QPcreJitStackPointer
{
    Q_DISABLE_COPY(QPcreJitStackPointer);

public:
    /*!
        \internal
    */
    QPcreJitStackPointer()
    {
        // The default JIT stack size in PCRE is 32K,
        // we allocate from 32K up to 512K.
        stack = pcre16_jit_stack_alloc(32*1024, 512*1024);
    }
    /*!
        \internal
    */
    ~QPcreJitStackPointer()
    {
        if (stack)
            pcre16_jit_stack_free(stack);
    }

    pcre16_jit_stack *stack;
};

Q_GLOBAL_STATIC(QThreadStorage<QPcreJitStackPointer *>, jitStacks)

/*!
    \internal
*/
static pcre16_jit_stack *qtPcreCallback(void *)
{
    if (jitStacks()->hasLocalData())
        return jitStacks()->localData()->stack;

    return 0;
}

/*!
    \internal
*/
static bool isJitEnabled()
{
    QByteArray jitEnvironment = qgetenv("QT_ENABLE_REGEXP_JIT");
    if (!jitEnvironment.isEmpty()) {
        bool ok;
        int enableJit = jitEnvironment.toInt(&ok);
        return ok ? (enableJit != 0) : true;
    }

#ifdef QT_DEBUG
    return false;
#else
    return true;
#endif
}

/*!
    \internal

    The purpose of the function is to call pcre16_study (which allows some
    optimizations to be performed, including JIT-compiling the pattern), and
    setting the studyData member variable to the result of the study. It gets
    called by doMatch() every time a match is performed. As of now, the
    optimizations on the pattern are performed after a certain number of usages
    (i.e. the qt_qregularexpression_optimize_after_use_count constant).

    Notice that although the method is protected by a mutex, one thread may
    invoke this function and return immediately (i.e. not study the pattern,
    leaving studyData to NULL); but before calling pcre16_exec to perform the
    match, another thread performs the studying and sets studyData to something
    else. Although the assignment to studyData is itself atomic, the release of
    the memory pointed by studyData isn't. Therefore, the current studyData
    value is returned and used by doMatch.
*/
pcre16_extra *QRegularExpressionPrivate::optimizePattern()
{
    Q_ASSERT(compiledPattern);

    QMutexLocker lock(&mutex);

    if (studyData || (++usedCount != qt_qregularexpression_optimize_after_use_count))
        return studyData;

    static const bool enableJit = isJitEnabled();

    int studyOptions = 0;
    if (enableJit)
        studyOptions |= PCRE_STUDY_JIT_COMPILE;

    const char *err;
    studyData = pcre16_study(compiledPattern, studyOptions, &err);

    if (studyData && studyData->flags & PCRE_EXTRA_EXECUTABLE_JIT)
        pcre16_assign_jit_stack(studyData, qtPcreCallback, 0);

    if (!studyData && err)
        qWarning("QRegularExpressionPrivate::optimizePattern(): pcre_study failed: %s", err);

    return studyData;
}

/*!
    \internal

    Returns the capturing group number for the given name. Duplicated names for
    capturing groups are not supported.
*/
int QRegularExpressionPrivate::captureIndexForName(const QString &name) const
{
    Q_ASSERT(!name.isEmpty());

    if (!compiledPattern)
        return -1;

    int index = pcre16_get_stringnumber(compiledPattern, name.utf16());
    if (index >= 0)
        return index;

    return -1;
}

/*!
    \internal

    This is a simple wrapper for pcre16_exec for handling the case in which the
    JIT runs out of memory. In that case, we allocate a thread-local JIT stack
    and re-run pcre16_exec.
*/
static int pcre16SafeExec(const pcre16 *code, const pcre16_extra *extra,
                          const unsigned short *subject, int length,
                          int startOffset, int options,
                          int *ovector, int ovecsize)
{
    int result = pcre16_exec(code, extra, subject, length,
                             startOffset, options, ovector, ovecsize);

    if (result == PCRE_ERROR_JIT_STACKLIMIT && !jitStacks()->hasLocalData()) {
        QPcreJitStackPointer *p = new QPcreJitStackPointer;
        jitStacks()->setLocalData(p);

        result = pcre16_exec(code, extra, subject, length,
                             startOffset, options, ovector, ovecsize);
    }

    return result;
}

/*!
    \internal

    Performs a match of type \a matchType on the given \a subject string with
    options \a matchOptions and returns the QRegularExpressionMatchPrivate of
    the result. It also advances a match if a previous result is given as \a
    previous. The \a subject string goes a Unicode validity check if
    \a checkSubjectString is true (PCRE doesn't like illegal UTF-16 sequences).

    Advancing a match is a tricky algorithm. If the previous match matched a
    non-empty string, we just do an ordinary match at the offset position.

    If the previous match matched an empty string, then an anchored, non-empty
    match is attempted at the offset position. If that succeeds, then we got
    the next match and we can return it. Otherwise, we advance by 1 position
    (which can be one or two code units in UTF-16!) and reattempt a "normal"
    match. We also have the problem of detecting the current newline format: if
    the new advanced offset is pointing to the beginning of a CRLF sequence, we
    must advance over it.
*/
QRegularExpressionMatchPrivate *QRegularExpressionPrivate::doMatch(const QString &subject,
                                                                   int offset,
                                                                   QRegularExpression::MatchType matchType,
                                                                   QRegularExpression::MatchOptions matchOptions,
                                                                   bool checkSubjectString,
                                                                   const QRegularExpressionMatchPrivate *previous) const
{
    if (offset < 0)
        offset += subject.length();

    QRegularExpression re(*const_cast<QRegularExpressionPrivate *>(this));

    if (offset < 0 || offset > subject.length())
        return new QRegularExpressionMatchPrivate(re, subject, matchType, matchOptions, 0);

    if (!compiledPattern) {
        qWarning("QRegularExpressionPrivate::doMatch(): called on an invalid QRegularExpression object");
        return new QRegularExpressionMatchPrivate(re, subject, matchType, matchOptions, 0);
    }

    QRegularExpressionMatchPrivate *priv = new QRegularExpressionMatchPrivate(re, subject,
                                                                              matchType, matchOptions,
                                                                              capturingCount);

    // this is mutex protected
    const pcre16_extra *currentStudyData = const_cast<QRegularExpressionPrivate *>(this)->optimizePattern();

    int pcreOptions = convertToPcreOptions(matchOptions);

    if (matchType == QRegularExpression::PartialPreferCompleteMatch)
        pcreOptions |= PCRE_PARTIAL_SOFT;
    else if (matchType == QRegularExpression::PartialPreferFirstMatch)
        pcreOptions |= PCRE_PARTIAL_HARD;

    if (!checkSubjectString)
        pcreOptions |= PCRE_NO_UTF16_CHECK;

    bool previousMatchWasEmpty = false;
    if (previous && previous->hasMatch &&
            (previous->capturedOffsets.at(0) == previous->capturedOffsets.at(1))) {
        previousMatchWasEmpty = true;
    }

    int * const captureOffsets = priv->capturedOffsets.data();
    const int captureOffsetsCount = priv->capturedOffsets.size();

    const unsigned short * const subjectUtf16 = subject.utf16();
    const int subjectLength = subject.length();

    int result;

    if (!previousMatchWasEmpty) {
        result = pcre16SafeExec(compiledPattern, currentStudyData,
                                subjectUtf16, subjectLength,
                                offset, pcreOptions,
                                captureOffsets, captureOffsetsCount);
    } else {
        result = pcre16SafeExec(compiledPattern, currentStudyData,
                                subjectUtf16, subjectLength,
                                offset, pcreOptions | PCRE_NOTEMPTY_ATSTART | PCRE_ANCHORED,
                                captureOffsets, captureOffsetsCount);

        if (result == PCRE_ERROR_NOMATCH) {
            ++offset;

            if (usingCrLfNewlines
                    && offset < subjectLength
                    && subjectUtf16[offset - 1] == QLatin1Char('\r')
                    && subjectUtf16[offset] == QLatin1Char('\n')) {
                ++offset;
            } else if (offset < subjectLength
                       && QChar::isLowSurrogate(subjectUtf16[offset])) {
                ++offset;
            }

            result = pcre16SafeExec(compiledPattern, currentStudyData,
                                    subjectUtf16, subjectLength,
                                    offset, pcreOptions,
                                    captureOffsets, captureOffsetsCount);
        }
    }

#ifdef QREGULAREXPRESSION_DEBUG
    qDebug() << "Matching" <<  pattern << "against" << subject
             << offset << matchType << matchOptions << previousMatchWasEmpty
             << "result" << result;
#endif

    // result == 0 means not enough space in captureOffsets; should never happen
    Q_ASSERT(result != 0);

    if (result > 0) {
        // full match
        priv->isValid = true;
        priv->hasMatch = true;
        priv->capturedCount = result;
        priv->capturedOffsets.resize(result * 2);
    } else {
        // no match, partial match or error
        priv->hasPartialMatch = (result == PCRE_ERROR_PARTIAL);
        priv->isValid = (result == PCRE_ERROR_NOMATCH || result == PCRE_ERROR_PARTIAL);

        if (result == PCRE_ERROR_PARTIAL) {
            // partial match:
            // leave the start and end capture offsets (i.e. cap(0))
            priv->capturedCount = 1;
            priv->capturedOffsets.resize(2);
        } else {
            // no match or error
            priv->capturedCount = 0;
            priv->capturedOffsets.clear();
        }
    }

    return priv;
}

/*!
    \internal
*/
QRegularExpressionMatchPrivate::QRegularExpressionMatchPrivate(const QRegularExpression &re,
                                                               const QString &subject,
                                                               QRegularExpression::MatchType matchType,
                                                               QRegularExpression::MatchOptions matchOptions,
                                                               int capturingCount)
    : regularExpression(re), subject(subject),
      matchType(matchType), matchOptions(matchOptions),
      capturedCount(0),
      hasMatch(false), hasPartialMatch(false), isValid(false)
{
    Q_ASSERT(capturingCount >= 0);
    const int captureOffsetsCount = (capturingCount + 1) * 3;
    capturedOffsets.resize(captureOffsetsCount);
}


/*!
    \internal
*/
QRegularExpressionMatch QRegularExpressionMatchPrivate::nextMatch() const
{
    Q_ASSERT(isValid);
    Q_ASSERT(hasMatch || hasPartialMatch);

    // Note the "false" passed for the check of the subject string:
    // if we're advancing a match on the same subject,
    // then that subject was already checked at least once (when this object
    // was created, or when the object that created this one was created, etc.)
    QRegularExpressionMatchPrivate *nextPrivate = regularExpression.d->doMatch(subject,
                                                                               capturedOffsets.at(1),
                                                                               matchType,
                                                                               matchOptions,
                                                                               false,
                                                                               this);
    return QRegularExpressionMatch(*nextPrivate);
}

/*!
    \internal
*/
QRegularExpressionMatchIteratorPrivate::QRegularExpressionMatchIteratorPrivate(const QRegularExpression &re,
                                                                               QRegularExpression::MatchType matchType,
                                                                               QRegularExpression::MatchOptions matchOptions,
                                                                               const QRegularExpressionMatch &next)
    : next(next),
      regularExpression(re),
      matchType(matchType), matchOptions(matchOptions)
{
}

/*!
    \internal
*/
bool QRegularExpressionMatchIteratorPrivate::hasNext() const
{
    return next.isValid() && (next.hasMatch() || next.hasPartialMatch());
}

// PUBLIC API

/*!
    Constructs a QRegularExpression object with an empty pattern and no pattern
    options.

    \sa setPattern(), setPatternOptions()
*/
QRegularExpression::QRegularExpression()
    : d(new QRegularExpressionPrivate)
{
}

/*!
    Constructs a QRegularExpression object using the given \a pattern as
    pattern and the \a options as the pattern options.

    \sa setPattern(), setPatternOptions()
*/
QRegularExpression::QRegularExpression(const QString &pattern, PatternOptions options)
    : d(new QRegularExpressionPrivate)
{
    d->pattern = pattern;
    d->patternOptions = options;
}

/*!
    Constructs a QRegularExpression object as a copy of \a re.

    \sa operator=()
*/
QRegularExpression::QRegularExpression(const QRegularExpression &re)
    : d(re.d)
{
}

/*!
    Destroys the QRegularExpression object.
*/
QRegularExpression::~QRegularExpression()
{
}

/*!
    Assigns the regular expression \a re to this object, and returns a reference
    to the copy. Both the pattern and the pattern options are copied.
*/
QRegularExpression &QRegularExpression::operator=(const QRegularExpression &re)
{
    d = re.d;
    return *this;
}

/*!
    \fn void QRegularExpression::swap(QRegularExpression &other)

    Swaps the regular expression \a other with this regular expression. This
    operation is very fast and never fails.
*/

/*!
    Returns the pattern string of the regular expression.

    \sa setPattern(), patternOptions()
*/
QString QRegularExpression::pattern() const
{
    return d->pattern;
}

/*!
    Sets the pattern string of the regular expression to \a pattern. The
    pattern options are left unchanged.

    \sa pattern(), setPatternOptions()
*/
void QRegularExpression::setPattern(const QString &pattern)
{
    d.detach();
    d->isDirty = true;
    d->pattern = pattern;
}

/*!
    Returns the pattern options for the regular expression.

    \sa setPatternOptions(), pattern()
*/
QRegularExpression::PatternOptions QRegularExpression::patternOptions() const
{
    return d->patternOptions;
}

/*!
    Sets the given \a options as the pattern options of the regular expression.
    The pattern string is left unchanged.

    \sa patternOptions(), setPattern()
*/
void QRegularExpression::setPatternOptions(PatternOptions options)
{
    d.detach();
    d->isDirty = true;
    d->patternOptions = options;
}

/*!
    Returns the number of capturing groups inside the pattern string,
    or -1 if the regular expression is not valid.

    \sa isValid()
*/
int QRegularExpression::captureCount() const
{
    if (!isValid()) // will compile the pattern
        return -1;
    return d->capturingCount;
}

/*!
    Returns true if the regular expression is a valid regular expression (that
    is, it contains no syntax errors, etc.), or false otherwise. Use
    errorString() to obtain a textual description of the error.

    \sa errorString(), patternErrorOffset()
*/
bool QRegularExpression::isValid() const
{
    d.data()->compilePattern();
    return d->compiledPattern;
}

/*!
    Returns a textual description of the error found when checking the validity
    of the regular expression, or "no error" if no error was found.

    \sa isValid(), patternErrorOffset()
*/
QString QRegularExpression::errorString() const
{
    d.data()->compilePattern();
    if (d->errorString)
        return QCoreApplication::translate("QRegularExpression", d->errorString);
    return QCoreApplication::translate("QRegularExpression", "no error");
}

/*!
    Returns the offset, inside the pattern string, at which an error was found
    when checking the validity of the regular expression. If no error was
    found, then -1 is returned.

    \sa pattern(), isValid(), errorString()
*/
int QRegularExpression::patternErrorOffset() const
{
    d.data()->compilePattern();
    return d->errorOffset;
}

/*!
    Attempts to match the regular expression against the given \a subject
    string, starting at the position \a offset inside the subject, using a
    match of type \a matchType and honoring the given \a matchOptions.

    The returned QRegularExpressionMatch object contains the results of the
    match.

    \sa QRegularExpressionMatch, {normal matching}
*/
QRegularExpressionMatch QRegularExpression::match(const QString &subject,
                                                  int offset,
                                                  MatchType matchType,
                                                  MatchOptions matchOptions) const
{
    d.data()->compilePattern();

    QRegularExpressionMatchPrivate *priv = d->doMatch(subject, offset, matchType, matchOptions);
    return QRegularExpressionMatch(*priv);
}

/*!
    Attempts to perform a global match of the regular expression against the
    given \a subject string, starting at the position \a offset inside the
    subject, using a match of type \a matchType and honoring the given \a
    matchOptions.

    The returned QRegularExpressionMatchIterator is positioned before the
    first match result (if any).

    \sa QRegularExpressionMatchIterator, {global matching}
*/
QRegularExpressionMatchIterator QRegularExpression::globalMatch(const QString &subject,
                                                                int offset,
                                                                MatchType matchType,
                                                                MatchOptions matchOptions) const
{
    QRegularExpressionMatchIteratorPrivate *priv =
            new QRegularExpressionMatchIteratorPrivate(*this,
                                                       matchType,
                                                       matchOptions,
                                                       match(subject, offset, matchType, matchOptions));

    return QRegularExpressionMatchIterator(*priv);
}

/*!
    Returns true if the regular expression is equal to \a re, or false
    otherwise. Two QRegularExpression objects are equal if they have
    the same pattern string and the same pattern options.

    \sa operator!=()
*/
bool QRegularExpression::operator==(const QRegularExpression &re) const
{
    return (d == re.d) ||
           (d->pattern == re.d->pattern && d->patternOptions == re.d->patternOptions);
}

/*!
    \fn bool QRegularExpression::operator!=(const QRegularExpression &re) const

    Returns true if the regular expression is different from \a re, or
    false otherwise.

    \sa operator==()
*/

/*!
    Escapes all characters of \a str so that they no longer have any special
    meaning when used as a regular expression pattern string, and returns
    the escaped string. For instance:

    \snippet code/src_corelib_tools_qregularexpression.cpp 26

    This is very convenient in order to build patterns from arbitrary strings:

    \snippet code/src_corelib_tools_qregularexpression.cpp 27

    \note This function implements Perl's quotemeta algorithm and escapes with
    a backslash all characters in \a str, except for the characters in the
    \c{[A-Z]}, \c{[a-z]} and \c{[0-9]} ranges, as well as the underscore
    (\c{_}) character. The only difference with Perl is that a literal NUL
    inside \a str is escaped with the sequence \c{"\\0"} (backslash +
    \c{'0'}), instead of \c{"\\\0"} (backslash + \c{NUL}).
*/
QString QRegularExpression::escape(const QString &str)
{
    QString result;
    const int count = str.size();
    result.reserve(count * 2);

    // everything but [a-zA-Z0-9_] gets escaped,
    // cf. perldoc -f quotemeta
    for (int i = 0; i < count; ++i) {
        const QChar current = str.at(i);

        if (current == QChar::Null) {
            // unlike Perl, a literal NUL must be escaped with
            // "\\0" (backslash + 0) and not "\\\0" (backslash + NUL),
            // because pcre16_compile uses a NUL-terminated string
            result.append(QLatin1Char('\\'));
            result.append(QLatin1Char('0'));
        } else if ( (current < QLatin1Char('a') || current > QLatin1Char('z')) &&
                    (current < QLatin1Char('A') || current > QLatin1Char('Z')) &&
                    (current < QLatin1Char('0') || current > QLatin1Char('9')) &&
                     current != QLatin1Char('_') )
        {
            result.append(QLatin1Char('\\'));
            result.append(current);
            if (current.isHighSurrogate() && i < (count - 1))
                result.append(str.at(++i));
        } else {
            result.append(current);
        }
    }

    result.squeeze();
    return result;
}

/*!
    Destroys the match result.
*/
QRegularExpressionMatch::~QRegularExpressionMatch()
{
}

/*!
    Constructs a match result by copying the result of the given \a match.

    \sa operator=()
*/
QRegularExpressionMatch::QRegularExpressionMatch(const QRegularExpressionMatch &match)
    : d(match.d)
{
}

/*!
    Assigns the match result \a match to this object, and returns a reference
    to the copy.
*/
QRegularExpressionMatch &QRegularExpressionMatch::operator=(const QRegularExpressionMatch &match)
{
    d = match.d;
    return *this;
}

/*!
    \fn void QRegularExpressionMatch::swap(QRegularExpressionMatch &other)

    Swaps the match result \a other with this match result. This
    operation is very fast and never fails.
*/

/*!
    \internal
*/
QRegularExpressionMatch::QRegularExpressionMatch(QRegularExpressionMatchPrivate &dd)
    : d(&dd)
{
}

/*!
    Returns the QRegularExpression object whose match() function returned this
    object.

    \sa QRegularExpression::match(), matchType(), matchOptions()
*/
QRegularExpression QRegularExpressionMatch::regularExpression() const
{
    return d->regularExpression;
}


/*!
    Returns the match type that was used to get this QRegularExpressionMatch
    object, that is, the match type that was passed to
    QRegularExpression::match() or QRegularExpression::globalMatch().

    \sa QRegularExpression::match(), regularExpression(), matchOptions()
*/
QRegularExpression::MatchType QRegularExpressionMatch::matchType() const
{
    return d->matchType;
}

/*!
    Returns the match options that were used to get this
    QRegularExpressionMatch object, that is, the match options that were passed
    to QRegularExpression::match() or QRegularExpression::globalMatch().

    \sa QRegularExpression::match(), regularExpression(), matchType()
*/
QRegularExpression::MatchOptions QRegularExpressionMatch::matchOptions() const
{
    return d->matchOptions;
}

/*!
    Returns the index of the last capturing group that captured something,
    including the implicit capturing group 0. This can be used to extract all
    the substrings that were captured:

    \snippet code/src_corelib_tools_qregularexpression.cpp 28

    Note that some of the capturing groups with an index less than
    lastCapturedIndex() could have not matched, and therefore captured nothing.

    If the regular expression did not match, this function returns -1.

    \sa captured(), capturedStart(), capturedEnd(), capturedLength()
*/
int QRegularExpressionMatch::lastCapturedIndex() const
{
    return d->capturedCount - 1;
}

/*!
    Returns the substring captured by the \a nth capturing group. If the \a nth
    capturing group did not capture a string or doesn't exist, returns a null
    QString.

    \sa capturedRef(), lastCapturedIndex(), capturedStart(), capturedEnd(),
    capturedLength(), QString::isNull()
*/
QString QRegularExpressionMatch::captured(int nth) const
{
    if (nth < 0 || nth > lastCapturedIndex())
        return QString();

    int start = capturedStart(nth);

    if (start == -1) // didn't capture
        return QString();

    return d->subject.mid(start, capturedLength(nth));
}

/*!
    Returns a reference to the substring captured by the \a nth capturing group.
    If the \a nth capturing group did not capture a string or doesn't exist,
    returns a null QStringRef.

    \sa captured(), lastCapturedIndex(), capturedStart(), capturedEnd(),
    capturedLength(), QStringRef::isNull()
*/
QStringRef QRegularExpressionMatch::capturedRef(int nth) const
{
    if (nth < 0 || nth > lastCapturedIndex())
        return QStringRef();

    int start = capturedStart(nth);

    if (start == -1) // didn't capture
        return QStringRef();

    return d->subject.midRef(start, capturedLength(nth));
}

/*!
    Returns the substring captured by the capturing group named \a name. If the
    capturing group named \a name did not capture a string or doesn't exist,
    returns a null QString.

    \sa capturedRef(), capturedStart(), capturedEnd(), capturedLength(),
    QString::isNull()
*/
QString QRegularExpressionMatch::captured(const QString &name) const
{
    if (name.isEmpty()) {
        qWarning("QRegularExpressionMatch::captured: empty capturing group name passed");
        return QString();
    }
    int nth = d->regularExpression.d->captureIndexForName(name);
    if (nth == -1)
        return QString();
    return captured(nth);
}

/*!
    Returns a reference to the string captured by the capturing group named \a
    name. If the capturing group named \a name did not capture a string or
    doesn't exist, returns a null QStringRef.

    \sa captured(), capturedStart(), capturedEnd(), capturedLength(),
    QStringRef::isNull()
*/
QStringRef QRegularExpressionMatch::capturedRef(const QString &name) const
{
    if (name.isEmpty()) {
        qWarning("QRegularExpressionMatch::capturedRef: empty capturing group name passed");
        return QStringRef();
    }
    int nth = d->regularExpression.d->captureIndexForName(name);
    if (nth == -1)
        return QStringRef();
    return capturedRef(nth);
}

/*!
    Returns a list of all strings captured by capturing groups, in the order
    the groups themselves appear in the pattern string.
*/
QStringList QRegularExpressionMatch::capturedTexts() const
{
    QStringList texts;
    for (int i = 0; i <= lastCapturedIndex(); ++i)
        texts << captured(i);
    return texts;
}

/*!
    Returns the offset inside the subject string corresponding to the
    starting position of the substring captured by the \a nth capturing group.
    If the \a nth capturing group did not capture a string or doesn't exist,
    returns -1.

    \sa capturedEnd(), capturedLength(), captured()
*/
int QRegularExpressionMatch::capturedStart(int nth) const
{
    if (nth < 0 || nth > lastCapturedIndex())
        return -1;

    return d->capturedOffsets.at(nth * 2);
}

/*!
    Returns the length of the substring captured by the \a nth capturing group.

    \note This function returns 0 if the \a nth capturing group did not capture
    a string or doesn't exist.

    \sa capturedStart(), capturedEnd(), captured()
*/
int QRegularExpressionMatch::capturedLength(int nth) const
{
    // bound checking performed by these two functions
    return capturedEnd(nth) - capturedStart(nth);
}

/*!
    Returns the offset inside the subject string immediately after the ending
    position of the substring captured by the \a nth capturing group. If the \a
    nth capturing group did not capture a string or doesn't exist, returns -1.

    \sa capturedStart(), capturedLength(), captured()
*/
int QRegularExpressionMatch::capturedEnd(int nth) const
{
    if (nth < 0 || nth > lastCapturedIndex())
        return -1;

    return d->capturedOffsets.at(nth * 2 + 1);
}

/*!
    Returns the offset inside the subject string corresponding to the starting
    position of the substring captured by the capturing group named \a name.
    If the capturing group named \a name did not capture a string or doesn't
    exist, returns -1.

    \sa capturedEnd(), capturedLength(), captured()
*/
int QRegularExpressionMatch::capturedStart(const QString &name) const
{
    if (name.isEmpty()) {
        qWarning("QRegularExpressionMatch::capturedStart: empty capturing group name passed");
        return -1;
    }
    int nth = d->regularExpression.d->captureIndexForName(name);
    if (nth == -1)
        return -1;
    return capturedStart(nth);
}

/*!
    Returns the offset inside the subject string corresponding to the starting
    position of the substring captured by the capturing group named \a name.

    \note This function returns 0 if the capturing group named \a name did not
    capture a string or doesn't exist.

    \sa capturedStart(), capturedEnd(), captured()
*/
int QRegularExpressionMatch::capturedLength(const QString &name) const
{
    if (name.isEmpty()) {
        qWarning("QRegularExpressionMatch::capturedLength: empty capturing group name passed");
        return 0;
    }
    int nth = d->regularExpression.d->captureIndexForName(name);
    if (nth == -1)
        return 0;
    return capturedLength(nth);
}

/*!
    Returns the offset inside the subject string immediately after the ending
    position of the substring captured by the capturing group named \a name. If
    the capturing group named \a name did not capture a string or doesn't
    exist, returns -1.

    \sa capturedStart(), capturedLength(), captured()
*/
int QRegularExpressionMatch::capturedEnd(const QString &name) const
{
    if (name.isEmpty()) {
        qWarning("QRegularExpressionMatch::capturedEnd: empty capturing group name passed");
        return -1;
    }
    int nth = d->regularExpression.d->captureIndexForName(name);
    if (nth == -1)
        return -1;
    return capturedEnd(nth);
}

/*!
    Returns true if the regular expression matched against the subject string,
    or false otherwise.

    \sa QRegularExpression::match(), hasPartialMatch()
*/
bool QRegularExpressionMatch::hasMatch() const
{
    return d->hasMatch;
}

/*!
    Returns true if the regular expression partially matched against the
    subject string, or false otherwise.

    \note Only a match that explicitely used the one of the partial match types
    can yield a partial match. Still, if such a match succeeds totally, this
    function will return false, while hasMatch() will return true.

    \sa QRegularExpression::match(), QRegularExpression::MatchType, hasMatch()
*/
bool QRegularExpressionMatch::hasPartialMatch() const
{
    return d->hasPartialMatch;
}

/*!
    Returns true if the match object was obtained as a result from the
    QRegularExpression::match() function invoked on a valid QRegularExpression
    object; returns false if the QRegularExpression was invalid.

    \sa QRegularExpression::match(), QRegularExpression::isValid()
*/
bool QRegularExpressionMatch::isValid() const
{
    return d->isValid;
}

/*!
    \internal
*/
QRegularExpressionMatchIterator::QRegularExpressionMatchIterator(QRegularExpressionMatchIteratorPrivate &dd)
    : d(&dd)
{
}

/*!
    Destroys the QRegularExpressionMatchIterator object.
*/
QRegularExpressionMatchIterator::~QRegularExpressionMatchIterator()
{
}

/*!
    Constructs a QRegularExpressionMatchIterator object as a copy of \a
    iterator.

    \sa operator=()
*/
QRegularExpressionMatchIterator::QRegularExpressionMatchIterator(const QRegularExpressionMatchIterator &iterator)
    : d(iterator.d)
{
}

/*!
    Assigns the iterator \a iterator to this object, and returns a reference to
    the copy.
*/
QRegularExpressionMatchIterator &QRegularExpressionMatchIterator::operator=(const QRegularExpressionMatchIterator &iterator)
{
    d = iterator.d;
    return *this;
}

/*!
    \fn void QRegularExpressionMatchIterator::swap(QRegularExpressionMatchIterator &other)

    Swaps the iterator \a other with this iterator object. This operation is
    very fast and never fails.
*/

/*!
    Returns true if the iterator object was obtained as a result from the
    QRegularExpression::globalMatch() function invoked on a valid
    QRegularExpression object; returns false if the QRegularExpression was
    invalid.

    \sa QRegularExpression::globalMatch(), QRegularExpression::isValid()
*/
bool QRegularExpressionMatchIterator::isValid() const
{
    return d->next.isValid();
}

/*!
    Returns true if there is at least one match result ahead of the iterator;
    otherwise it returns false.

    \sa next()
*/
bool QRegularExpressionMatchIterator::hasNext() const
{
    return d->hasNext();
}

/*!
    Returns the next match result without moving the iterator.

    \note Calling this function when the iterator is at the end of the result
    set leads to undefined results.
*/
QRegularExpressionMatch QRegularExpressionMatchIterator::peekNext() const
{
    if (!hasNext())
        qWarning("QRegularExpressionMatchIterator::peekNext() called on an iterator already at end");

    return d->next;
}

/*!
    Returns the next match result and advances the iterator by one position.

    \note Calling this function when the iterator is at the end of the result
    set leads to undefined results.
*/
QRegularExpressionMatch QRegularExpressionMatchIterator::next()
{
    if (!hasNext()) {
        qWarning("QRegularExpressionMatchIterator::next() called on an iterator already at end");
        return d->next;
    }

    QRegularExpressionMatch current = d->next;
    d->next = d->next.d.constData()->nextMatch();
    return current;
}

/*!
    Returns the QRegularExpression object whose globalMatch() function returned
    this object.

    \sa QRegularExpression::globalMatch(), matchType(), matchOptions()
*/
QRegularExpression QRegularExpressionMatchIterator::regularExpression() const
{
    return d->regularExpression;
}

/*!
    Returns the match type that was used to get this
    QRegularExpressionMatchIterator object, that is, the match type that was
    passed to QRegularExpression::globalMatch().

    \sa QRegularExpression::globalMatch(), regularExpression(), matchOptions()
*/
QRegularExpression::MatchType QRegularExpressionMatchIterator::matchType() const
{
    return d->matchType;
}

/*!
    Returns the match options that were used to get this
    QRegularExpressionMatchIterator object, that is, the match options that
    were passed to QRegularExpression::globalMatch().

    \sa QRegularExpression::globalMatch(), regularExpression(), matchType()
*/
QRegularExpression::MatchOptions QRegularExpressionMatchIterator::matchOptions() const
{
    return d->matchOptions;
}

#ifndef QT_NO_DATASTREAM
/*!
    \relates QRegularExpression

    Writes the regular expression \a re to stream \a out.

    \sa {Serializing Qt Data Types}
*/
QDataStream &operator<<(QDataStream &out, const QRegularExpression &re)
{
    out << re.pattern() << quint32(re.patternOptions());
    return out;
}

/*!
    \relates QRegularExpression

    Reads a regular expression from stream \a in into \a re.

    \sa {Serializing Qt Data Types}
*/
QDataStream &operator>>(QDataStream &in, QRegularExpression &re)
{
    QString pattern;
    quint32 patternOptions;
    in >> pattern >> patternOptions;
    re.setPattern(pattern);
    re.setPatternOptions(QRegularExpression::PatternOptions(patternOptions));
    return in;
}
#endif

#ifndef QT_NO_DEBUG_STREAM
/*!
    \relates QRegularExpression

    Writes the regular expression \a re into the debug object \a debug for
    debugging purposes.

    \sa {Debugging Techniques}
*/
QDebug operator<<(QDebug debug, const QRegularExpression &re)
{
    debug.nospace() << "QRegularExpression(" << re.pattern() << ", " << re.patternOptions() << ")";
    return debug.space();
}

/*!
    \relates QRegularExpression

    Writes the pattern options \a patternOptions into the debug object \a debug
    for debugging purposes.

    \sa {Debugging Techniques}
*/
QDebug operator<<(QDebug debug, QRegularExpression::PatternOptions patternOptions)
{
    QByteArray flags;

    if (patternOptions == QRegularExpression::NoPatternOption) {
        flags = "NoPatternOption";
    } else {
        flags.reserve(200); // worst case...
        if (patternOptions & QRegularExpression::CaseInsensitiveOption)
            flags.append("CaseInsensitiveOption|");
        if (patternOptions & QRegularExpression::DotMatchesEverythingOption)
            flags.append("DotMatchesEverythingOption|");
        if (patternOptions & QRegularExpression::MultilineOption)
            flags.append("MultilineOption|");
        if (patternOptions & QRegularExpression::ExtendedPatternSyntaxOption)
            flags.append("ExtendedPatternSyntaxOption|");
        if (patternOptions & QRegularExpression::InvertedGreedinessOption)
            flags.append("InvertedGreedinessOption|");
        if (patternOptions & QRegularExpression::DontCaptureOption)
            flags.append("DontCaptureOption|");
        if (patternOptions & QRegularExpression::UseUnicodePropertiesOption)
            flags.append("UseUnicodePropertiesOption|");
        flags.chop(1);
    }

    debug.nospace() << "QRegularExpression::PatternOptions(" << flags << ")";

    return debug.space();
}
/*!
    \relates QRegularExpressionMatch

    Writes the match object \a match into the debug object \a debug for
    debugging purposes.

    \sa {Debugging Techniques}
*/
QDebug operator<<(QDebug debug, const QRegularExpressionMatch &match)
{
    debug.nospace() << "QRegularExpressionMatch(";

    if (!match.isValid()) {
        debug << "Invalid)";
        return debug.space();
    }

    debug << "Valid";

    if (match.hasMatch()) {
        debug << ", has match: ";
        for (int i = 0; i <= match.lastCapturedIndex(); ++i) {
            debug << i
                  << ":(" << match.capturedStart(i) << ", " << match.capturedEnd(i)
                  << ", " << match.captured(i) << ")";
            if (i < match.lastCapturedIndex())
                debug << ", ";
        }
    } else if (match.hasPartialMatch()) {
        debug << ", has partial match: ("
              << match.capturedStart(0) << ", "
              << match.capturedEnd(0) << ", "
              << match.captured(0) << ")";
    } else {
        debug << ", no match";
    }

    debug << ")";

    return debug.space();
}
#endif

// fool lupdate: make it extract those strings for translation, but don't put them
// inside Qt -- they're already inside libpcre (cf. man 3 pcreapi, pcre_compile.c).
#if 0

/* PCRE is a library of functions to support regular expressions whose syntax
and semantics are as close as possible to those of the Perl 5 language.

                       Written by Philip Hazel
           Copyright (c) 1997-2012 University of Cambridge

-----------------------------------------------------------------------------
Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice,
      this list of conditions and the following disclaimer.

    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.

    * Neither the name of the University of Cambridge nor the names of its
      contributors may be used to endorse or promote products derived from
      this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.
-----------------------------------------------------------------------------
*/

static const char *pcreCompileErrorCodes[] =
{
    QT_TRANSLATE_NOOP("QRegularExpression", "no error"),
    QT_TRANSLATE_NOOP("QRegularExpression", "\\ at end of pattern"),
    QT_TRANSLATE_NOOP("QRegularExpression", "\\c at end of pattern"),
    QT_TRANSLATE_NOOP("QRegularExpression", "unrecognized character follows \\"),
    QT_TRANSLATE_NOOP("QRegularExpression", "numbers out of order in {} quantifier"),
    QT_TRANSLATE_NOOP("QRegularExpression", "number too big in {} quantifier"),
    QT_TRANSLATE_NOOP("QRegularExpression", "missing terminating ] for character class"),
    QT_TRANSLATE_NOOP("QRegularExpression", "invalid escape sequence in character class"),
    QT_TRANSLATE_NOOP("QRegularExpression", "range out of order in character class"),
    QT_TRANSLATE_NOOP("QRegularExpression", "nothing to repeat"),
    QT_TRANSLATE_NOOP("QRegularExpression", "internal error: unexpected repeat"),
    QT_TRANSLATE_NOOP("QRegularExpression", "unrecognized character after (? or (?-"),
    QT_TRANSLATE_NOOP("QRegularExpression", "POSIX named classes are supported only within a class"),
    QT_TRANSLATE_NOOP("QRegularExpression", "missing )"),
    QT_TRANSLATE_NOOP("QRegularExpression", "reference to non-existent subpattern"),
    QT_TRANSLATE_NOOP("QRegularExpression", "erroffset passed as NULL"),
    QT_TRANSLATE_NOOP("QRegularExpression", "unknown option bit(s) set"),
    QT_TRANSLATE_NOOP("QRegularExpression", "missing ) after comment"),
    QT_TRANSLATE_NOOP("QRegularExpression", "regular expression is too large"),
    QT_TRANSLATE_NOOP("QRegularExpression", "failed to get memory"),
    QT_TRANSLATE_NOOP("QRegularExpression", "unmatched parentheses"),
    QT_TRANSLATE_NOOP("QRegularExpression", "internal error: code overflow"),
    QT_TRANSLATE_NOOP("QRegularExpression", "unrecognized character after (?<"),
    QT_TRANSLATE_NOOP("QRegularExpression", "lookbehind assertion is not fixed length"),
    QT_TRANSLATE_NOOP("QRegularExpression", "malformed number or name after (?("),
    QT_TRANSLATE_NOOP("QRegularExpression", "conditional group contains more than two branches"),
    QT_TRANSLATE_NOOP("QRegularExpression", "assertion expected after (?("),
    QT_TRANSLATE_NOOP("QRegularExpression", "(?R or (?[+-]digits must be followed by )"),
    QT_TRANSLATE_NOOP("QRegularExpression", "unknown POSIX class name"),
    QT_TRANSLATE_NOOP("QRegularExpression", "POSIX collating elements are not supported"),
    QT_TRANSLATE_NOOP("QRegularExpression", "this version of PCRE is not compiled with PCRE_UTF8 support"),
    QT_TRANSLATE_NOOP("QRegularExpression", "character value in \\x{...} sequence is too large"),
    QT_TRANSLATE_NOOP("QRegularExpression", "invalid condition (?(0)"),
    QT_TRANSLATE_NOOP("QRegularExpression", "\\C not allowed in lookbehind assertion"),
    QT_TRANSLATE_NOOP("QRegularExpression", "PCRE does not support \\L, \\l, \\N{name}, \\U, or \\u"),
    QT_TRANSLATE_NOOP("QRegularExpression", "number after (?C is > 255"),
    QT_TRANSLATE_NOOP("QRegularExpression", "closing ) for (?C expected"),
    QT_TRANSLATE_NOOP("QRegularExpression", "recursive call could loop indefinitely"),
    QT_TRANSLATE_NOOP("QRegularExpression", "unrecognized character after (?P"),
    QT_TRANSLATE_NOOP("QRegularExpression", "syntax error in subpattern name (missing terminator)"),
    QT_TRANSLATE_NOOP("QRegularExpression", "two named subpatterns have the same name"),
    QT_TRANSLATE_NOOP("QRegularExpression", "invalid UTF-8 string"),
    QT_TRANSLATE_NOOP("QRegularExpression", "support for \\P, \\p, and \\X has not been compiled"),
    QT_TRANSLATE_NOOP("QRegularExpression", "malformed \\P or \\p sequence"),
    QT_TRANSLATE_NOOP("QRegularExpression", "unknown property name after \\P or \\p"),
    QT_TRANSLATE_NOOP("QRegularExpression", "subpattern name is too long (maximum 32 characters)"),
    QT_TRANSLATE_NOOP("QRegularExpression", "too many named subpatterns (maximum 10000)"),
    QT_TRANSLATE_NOOP("QRegularExpression", "octal value is greater than \\377 (not in UTF-8 mode)"),
    QT_TRANSLATE_NOOP("QRegularExpression", "internal error: overran compiling workspace"),
    QT_TRANSLATE_NOOP("QRegularExpression", "internal error: previously-checked referenced subpattern not found"),
    QT_TRANSLATE_NOOP("QRegularExpression", "DEFINE group contains more than one branch"),
    QT_TRANSLATE_NOOP("QRegularExpression", "repeating a DEFINE group is not allowed"),
    QT_TRANSLATE_NOOP("QRegularExpression", "inconsistent NEWLINE options"),
    QT_TRANSLATE_NOOP("QRegularExpression", "\\g is not followed by a braced, angle-bracketed, or quoted name/number or by a plain number"),
    QT_TRANSLATE_NOOP("QRegularExpression", "a numbered reference must not be zero"),
    QT_TRANSLATE_NOOP("QRegularExpression", "an argument is not allowed for (*ACCEPT), (*FAIL), or (*COMMIT)"),
    QT_TRANSLATE_NOOP("QRegularExpression", "(*VERB) not recognized"),
    QT_TRANSLATE_NOOP("QRegularExpression", "number is too big"),
    QT_TRANSLATE_NOOP("QRegularExpression", "subpattern name expected"),
    QT_TRANSLATE_NOOP("QRegularExpression", "digit expected after (?+"),
    QT_TRANSLATE_NOOP("QRegularExpression", "] is an invalid data character in JavaScript compatibility mode"),
    QT_TRANSLATE_NOOP("QRegularExpression", "different names for subpatterns of the same number are not allowed"),
    QT_TRANSLATE_NOOP("QRegularExpression", "(*MARK) must have an argument"),
    QT_TRANSLATE_NOOP("QRegularExpression", "this version of PCRE is not compiled with PCRE_UCP support"),
    QT_TRANSLATE_NOOP("QRegularExpression", "\\c must be followed by an ASCII character"),
    QT_TRANSLATE_NOOP("QRegularExpression", "\\k is not followed by a braced, angle-bracketed, or quoted name"),
    QT_TRANSLATE_NOOP("QRegularExpression", "internal error: unknown opcode in find_fixedlength()"),
    QT_TRANSLATE_NOOP("QRegularExpression", "\\N is not supported in a class"),
    QT_TRANSLATE_NOOP("QRegularExpression", "too many forward references"),
    QT_TRANSLATE_NOOP("QRegularExpression", "disallowed Unicode code point (>= 0xd800 && <= 0xdfff)"),
    QT_TRANSLATE_NOOP("QRegularExpression", "invalid UTF-16 string")
};
#endif // #if 0
