#ifndef SPELLCHECKER_H
#define SPELLCHECKER_H

#include <QString>
#include <QTextBoundaryFinder>

class Hunspell;

struct SpellCheckResult
{
    int start;
    int end;
};

typedef QList<SpellCheckResult> SpellCheckResultList;

class SpellCheckTokenizer
{
public:
    SpellCheckTokenizer(const QString *text);

    bool hasNextWord();
    QString getNextWord() const;
    int nextWordStart() const;
    int nextWordEnd() const;

private:
    const QString *text;
    QTextBoundaryFinder boundaryFinder;
    int position;
    int startWord;
    int start, end;
    QString nextWord;
};

class SpellChecker
{
public:
    SpellChecker(const QString &dictionaryPath, const QString &userDictionary, const QString &lan);
    ~SpellChecker();

    bool spell(const QString &word);
    SpellCheckResultList checkString(const QString &text);
    QStringList suggest(const QString &word);
    void ignoreWord(const QString &word);
    void addToUserWordlist(const QString &word);
    QString getLan();

private:

private:
    void put_word(const QString &word);
    Hunspell *hunspell;
    QString userDictionary;
    QString encoding;
    QTextCodec *codec;
    QString lan;
};

#endif
