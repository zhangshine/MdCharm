#include <cassert>

#include <QApplication>

#include "hightlighter.h"
#include "util/test/qregularexpression.h"

HighLighter::HighLighter(QTextDocument *parent) :
    QSyntaxHighlighter(parent)
{
}

HighLighter::HighLighter(QTextEdit *parent) :
    QSyntaxHighlighter(parent)
{

}

void HighLighter::addTextCharFormat(const int weight, const QColor color, bool isItalic,
                                    const QString &regExp)
{
    HighlightingRule rule;
    QTextCharFormat format;
    if(weight!=QFont::Normal)
        format.setFontWeight(weight);
    if(color.isValid())
        format.setForeground(color);
    if(isItalic)
        format.setFontItalic(isItalic);
    rule.pattern = regExp;
    rule.format = format;
    highlightingRules.append(rule);
}

void HighLighter::highlightBlock(const QString &text)
{
    foreach(const HighlightingRule &rule, highlightingRules)
    {
        QRegularExpression re(rule.pattern, QRegularExpression::DotMatchesEverythingOption);
#ifdef MDCHARM_DEBUG
        if(!re.isValid())
        {
            qDebug(rule.pattern.toLatin1());
        }
#endif
        QRegularExpressionMatchIterator remi = re.globalMatch(text);
        while(remi.hasNext())
        {
            QRegularExpressionMatch rem = remi.next();
            setFormat(rem.capturedStart(), rem.capturedLength(), rule.format);
        }
    }

    setCurrentBlockState(0);
    highlightMultiLine(text);
}

void HighLighter::highlightMultiLine(const QString &text)
{
    Q_UNUSED(text)
    return;
}

MarkdownHighLighter::MarkdownHighLighter(QTextDocument *parent) :
    HighLighter(parent)
{
    //html tags
    addTextCharFormat(QFont::Bold, Qt::darkMagenta, false, QString::fromLatin1("<[^<>@]*>"));
    //html symbols
    addTextCharFormat(QFont::Bold, Qt::darkCyan, false, QString::fromLatin1("&[^; ]*;"));
    //quote inside tag
    addTextCharFormat(QFont::Bold, Qt::darkYellow, false, QString::fromLatin1("\"[^\"<]*\"(?=[^<]*>)"));
    //html comment
    addTextCharFormat(QFont::Bold, Qt::gray, false, QString::fromLatin1("<!--[^<>]*-->"));
    //italic
    addTextCharFormat(QFont::Normal, Qt::darkCyan, true, QString::fromLatin1("(\\s|^)[\\*_]{1}[^\\s]{1}[^\\*_]+[\\*_]{1}(\\s|\\.|,|;|:|\\-|\\?|$)"));
    //bold
    addTextCharFormat(QFont::Bold, Qt::darkCyan, false, QString::fromLatin1("(\\s|^)[\\*_]{2}[^\\s]{1}[^\\*_]+[\\*_]{2}(\\s|\\.|,|;|:|\\-|\\?|$)"));
    //bold and italic
    addTextCharFormat(QFont::Bold, Qt::darkCyan, true, QString::fromLatin1("(\\s|^)[\\*_]{3}[^\\*_]+[\\*_]{3}(\\s|\\.|,|;|:|\\-|\\?|$)"));
    //link and images
    addTextCharFormat(QFont::Normal, Qt::blue, false, QString::fromLatin1("(?<=\\[)[^\\[\\]]*(?=\\])"));
    addTextCharFormat(QFont::Normal, Qt::blue, false, QString::fromLatin1("(?<=\\]\\()[^\\(\\)]*(?=\\))"));
    //blockquote
    addTextCharFormat(QFont::Normal, Qt::darkGray, false, QString::fromLatin1("^\\s{0,3}>"));
    //header
    addTextCharFormat(QFont::Bold, Qt::darkMagenta, false, QString::fromLatin1("^#.*$"));
    addTextCharFormat(QFont::Bold, Qt::darkMagenta, false, QString::fromLatin1("^==+$"));
    //bullet
    addTextCharFormat(QFont::Normal, Qt::darkRed, false, QString::fromLatin1("^[\\*\\+\\-]\\s"));
    //code
    addTextCharFormat(QFont::Normal, Qt::darkBlue, false, QString::fromLatin1("^([\\s]{4,}|\\t+).*$"));
}

MarkdownHighLighter::~MarkdownHighLighter()
{
}

CSSHighLighter::CSSHighLighter(QTextDocument *parent) :
    HighLighter(parent)
{
    addTextCharFormat(QFont::Normal, Qt::red, false, QString::fromLatin1("-?[A-Za-z_-]+(?=\\s*:)"));
    //id
    addTextCharFormat(QFont::Normal, Qt::darkCyan, false, QString::fromLatin1("#([a-zA-Z0-9\\-_]|[\\x80-\\xFF]|\\\\[0-9A-Fa-f]{1,6})*"));
    //class
    addTextCharFormat(QFont::Normal, Qt::darkCyan, false, QString::fromLatin1("(?<= |^)\\.([a-zA-Z0-9\\-_]|[\\x80-\\xFF]|\\\\[0-9A-Fa-f]{1,6})*"));
    //number
    addTextCharFormat(QFont::Normal, Qt::darkBlue, false, QString::fromLatin1("(?<=:)[ 0-9.%]*"));
    //value
    addTextCharFormat(QFont::Normal, Qt::darkGreen, false, QString::fromLatin1("[-+]?[0-9.]+(em|ex|ch|rem|vw|vh|vm|px|in|cm|mm|pt|pc|deg|rad|grad|turn|ms|s|Hz|kHz)\\b"));

    commentsFormat.setForeground(Qt::darkGreen);
    commentsStart = QRegExp("/\\*");
    commentsEnd = QRegExp("\\*/");
}

void CSSHighLighter::highlightMultiLine(const QString &text)
{
    int startIndex = 0;
    if(previousBlockState() != 1)
        startIndex = commentsStart.indexIn(text);
    while (startIndex >= 0)
    {
        int endIndex = commentsEnd.indexIn(text, startIndex);
        int commentLength;
        if (endIndex == -1)
        {
            setCurrentBlockState(1);
            commentLength = text.length() - startIndex;
        }
        else
        {
            commentLength = endIndex - startIndex
                    + commentsEnd.matchedLength();
        }
        setFormat(startIndex, commentLength, commentsFormat);
        startIndex = commentsStart.indexIn(text, startIndex + commentLength);
    }
}
