#ifndef HIGHTLIGHTER_H
#define HIGHTLIGHTER_H

#include <QSyntaxHighlighter>

#ifdef QT_V5
#include <QtWidgets>
#endif

class QTextDocument;

struct HighlightingRule
{
    QString pattern;
    QTextCharFormat format;
};

class HighLighter : public QSyntaxHighlighter
{
    Q_OBJECT
public:
    HighLighter(QTextDocument *parent);
    HighLighter(QTextEdit *parent);
    void addTextCharFormat(const int weight, const QColor color, bool isItalic,
                           const QString &regExp);
protected:
    virtual void highlightBlock(const QString &text);
    virtual void highlightMultiLine(const QString &text);
protected:
    QVector<HighlightingRule> highlightingRules;
};

class MarkdownHighLighter: public HighLighter
{
    Q_OBJECT
public:
    MarkdownHighLighter(QTextDocument *parent = 0);
    ~MarkdownHighLighter();
};

class CSSHighLighter: public HighLighter
{
    Q_OBJECT
public:
    CSSHighLighter(QTextDocument *parent = 0);
protected:
    virtual void highlightMultiLine(const QString &text);
private:
    QTextCharFormat commentsFormat;
    QRegExp commentsStart;
    QRegExp commentsEnd;
};

#endif // HIGHTLIGHTER_H
