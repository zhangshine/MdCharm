#include "spellchecker.h"
#include "utils.h"
#include "configuration.h"
#include "hunspell.hxx"

#include <QFile>
#include <QTextStream>
#include <QRegExp>
#include <QTextCodec>
#include <QStringList>

SpellCheckTokenizer::SpellCheckTokenizer(const QString *text)
{
    this->text = text;
    boundaryFinder = QTextBoundaryFinder(QTextBoundaryFinder::Word, *text);
    position = 0;
    startWord = -1;
#ifdef QT_V5
    if(boundaryFinder.boundaryReasons() & QTextBoundaryFinder::StartOfItem)
#else
    if(boundaryFinder.boundaryReasons() & QTextBoundaryFinder::StartWord)
#endif
        startWord = 0;
}

bool SpellCheckTokenizer::hasNextWord()
{
    nextWord.clear();
    while(true){
        position = boundaryFinder.toNextBoundary();
        if(position==-1)
            return false;

        const QTextBoundaryFinder::BoundaryReasons reasons = boundaryFinder.boundaryReasons();
#ifdef QT_V5
        if(reasons & QTextBoundaryFinder::EndOfItem){
#else
        if(reasons & QTextBoundaryFinder::EndWord){
#endif
            if(startWord==-1)
                continue;
            //skip empty words
            if(position==startWord)
                continue;
            nextWord = text->mid(startWord, position-startWord);
            start=startWord;
            end=position;
#ifdef QT_V5
            if(reasons & QTextBoundaryFinder::StartOfItem)
#else
            if(reasons & QTextBoundaryFinder::StartWord)
#endif
                startWord = position;
            else
                startWord = -1;
            return true;
        }
#ifdef QT_V5
        if(reasons & QTextBoundaryFinder::StartOfItem)
#else
        if(reasons & QTextBoundaryFinder::StartWord)
#endif
            startWord = position;
    }
    return false;
}

QString SpellCheckTokenizer::getNextWord() const
{
    return nextWord;
}

int SpellCheckTokenizer::nextWordStart() const
{
    return start;
}

int SpellCheckTokenizer::nextWordEnd() const
{
    return end;
}

//SpellChecker *SpellChecker::spellCheckInstance = NULL;

//SpellChecker* SpellChecker::getInstance()
//{
//    Configuration *conf = Configuration::getInstance();
//    if(!spellCheckInstance && conf->isCheckSpell()){
//        QString filePath = conf->getLanguageSpellCheckDictPath(conf->getSpellCheckLanguage());
//        if(!filePath.isEmpty()){
//            QString withoutSuffixFilePath = filePath.left(filePath.lastIndexOf('.'));
//            qDebug(withoutSuffixFilePath.toAscii());
//            spellCheckInstance = new SpellChecker(withoutSuffixFilePath, conf->getLanguageSpellCheckUserDictPath(), conf->getSpellCheckLanguage());
//        } else {
//            conf->setCheckSpell(false);
//            conf->setSpellCheckLanguage(QString());
//            spellCheckInstance = NULL;
//        }
//    }
//    return spellCheckInstance;
//}

//void SpellChecker::reload()
//{
//    Configuration *conf = Configuration::getInstance();

//    if(conf->isCheckSpell()){
//        if(spellCheckInstance!=NULL&&spellCheckInstance->getLan()!=conf->getSpellCheckLanguage()){
//            delete spellCheckInstance;
//            spellCheckInstance = NULL;
//        }
//        getInstance();
//        return;
//    }else{
//        delete spellCheckInstance;
//        spellCheckInstance = NULL;
//        return;
//    }
//}

SpellChecker::SpellChecker(const QString &dictionaryPath, const QString &userDictionary, const QString &lan)
{
    this->lan = lan;
    this->userDictionary = userDictionary;
    QString dictFilePath = dictionaryPath + ".dic";
    QString affixFilePath = dictionaryPath + ".aff";
    QByteArray dictFilePathBA = dictFilePath.toLocal8Bit();
    QByteArray affixFilePathBA = affixFilePath.toLocal8Bit();
    hunspell = new Hunspell(affixFilePathBA.constData(), dictFilePathBA.constData());

    encoding = "ISO8859-1";
    QFile affixFile(affixFilePath);
    if(affixFile.open(QIODevice::ReadOnly)) {
        QTextStream stream(&affixFile);
        QRegExp enc_detector("^\\s*SET\\s+([A-Z0-9\\-]+)\\s*", Qt::CaseInsensitive);
        for(QString line = stream.readLine(); !line.isEmpty(); line=stream.readLine()) {
            if(enc_detector.indexIn(line) > -1) {
                encoding = enc_detector.cap(1);
                qDebug("Encoding set to %s", encoding.toLocal8Bit().constData());
                break;
            }
        }
        affixFile.close();
    }
    codec = QTextCodec::codecForName(encoding.toLatin1().constData());
    if(userDictionary.isEmpty()) {
        QFile userDictionaryFile(userDictionary);
        if(userDictionaryFile.open(QIODevice::ReadOnly)) {
            QTextStream stream(&userDictionaryFile);
            for(QString word=stream.readLine(); !word.isEmpty(); word=stream.readLine())
                put_word(word);
            userDictionaryFile.close();
        } else {
            qWarning("User dictionary in %s could not be opened", userDictionary.toLocal8Bit().constData());
        }
    } else {
        qDebug("User dictionary not set.");
    }
}

SpellChecker::~SpellChecker()
{
    delete hunspell;
}

bool SpellChecker::spell(const QString &word)
{
    return hunspell->spell(codec->fromUnicode(word).constData()) != 0;
}

SpellCheckResultList SpellChecker::checkString(const QString &text)
{
    SpellCheckResultList resultList;
    if (text.isEmpty())
        return resultList;//Found 0 error
    SpellCheckTokenizer tokenizer(&text);
    while(tokenizer.hasNextWord()){
        QString word = tokenizer.getNextWord();
        if(!Utils::isLetterOrNumberString(word))
            continue;
        //only accept alhpanumeric string
        if(!hunspell->spell(codec->fromUnicode(word))){
            SpellCheckResult result;
            result.start = tokenizer.nextWordStart();
            result.end = tokenizer.nextWordEnd();
            resultList.append(result);
        }
    }
    return resultList;
}

QStringList SpellChecker::suggest(const QString &word)
{
    char **suggestWordList;

    int numSuggestions = hunspell->suggest(&suggestWordList, codec->fromUnicode(word).constData());
    QStringList suggestions;
    for(int i=0; i<numSuggestions; ++i) {
        suggestions << codec->toUnicode(suggestWordList[i]);
        free(suggestWordList[i]);
    }
    return suggestions;
}

void SpellChecker::ignoreWord(const QString &word)
{
    put_word(word);
}

void SpellChecker::put_word(const QString &word)
{
    hunspell->add(codec->fromUnicode(word).constData());
}

void SpellChecker::addToUserWordlist(const QString &word)
{
    put_word(word);
    if(userDictionary.isEmpty()) {
        QFile userDictionaryFile(userDictionary);
        if(userDictionaryFile.open(QIODevice::Append)) {
            QTextStream stream(&userDictionaryFile);
            stream << word << "\n";
            userDictionaryFile.close();
        } else {
            qWarning("User dictionary in %s could not be opened for appending a new word", userDictionary.toLocal8Bit().constData());
        }
    } else {
        qDebug("User dictionary not set.");
    }
}

QString SpellChecker::getLan()
{
    return lan;
}
