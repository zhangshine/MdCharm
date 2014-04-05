#ifndef BASEEDITOR_H
#define BASEEDITOR_H

#include <QPlainTextEdit>

class Configuration;
class MdCharmGlobal;

class BaseEditor : public QPlainTextEdit
{
    Q_OBJECT

public:
    explicit BaseEditor(QWidget *parent = 0);
    ~BaseEditor();
    void lineNumberAreaPaintEvent(QPaintEvent *event);
    int lineNumberAreaWidth();
    int firstVisibleLineNumber();
    void enableHighlightCurrentLine();
    void disableHighlightCurrentLine();
    void enableDisplayLineNumber();
    void disableDisplayLineNumber();
    void enableSpellCheck();
    void disableSpellCheck();
    void setDocument(QTextDocument *doc);
protected:
    virtual void resizeEvent(QResizeEvent *e);
    virtual void keyPressEvent(QKeyEvent *e);
    virtual void focusInEvent(QFocusEvent *e);
    virtual void paintEvent(QPaintEvent *e);
signals:
    void overWriteModeChanged();
    void focusInSignal();

public slots:
    void findAndHighlightText(const QString &text, QTextDocument::FindFlags qff,
                              bool isRE, bool isSetTextCursor=true);
    void findFirstOccurrance(const QString &text, QTextDocument::FindFlags qff,
                             bool isRE, bool init=false, bool isSetTextCusor=true);
    void findFinished();
    void replace(const QString &rt);
    void replaceAll(const QString &ft, const QString &rt,
                    QTextDocument::FindFlags qff, bool isRE);
private slots:
    void updateLineNumberAreaWidth(int newBlockCount);
    void highlightCurrentLine();
    void updateLineNumberArea(const QRect &, int);
    void ensureAtTheLast();
    void spellCheck(int start, int unused, int length);
private:
    void initSpellCheckMatter();
    void updateExtraSelection();
    bool findAllOccurrance(const QString &text, QTextDocument::FindFlags qff, bool isRE);
    void spellCheckAux(const QTextBlock &block);
    void checkWholeContent();
    void removeExtraSelectionInRange(QList<QTextEdit::ExtraSelection> &extraList, int start, int end);
protected:
     Configuration *conf;
     MdCharmGlobal *mdCharmGlobal;
     QString spellCheckLanguage;
private:
    QWidget *lineNumberArea;
    bool displayLineNumber;
    QList<QTextEdit::ExtraSelection> currentLineSelection;
    QList<QTextEdit::ExtraSelection> findTextSelection;
    QList<QTextEdit::ExtraSelection> currentFindSelection;
    QList<QTextEdit::ExtraSelection> spellCheckErrorSelection;
    QTextCursor prevFindCursor;
    bool finded;
    bool replacing;
};

class LineNumberArea : public QWidget
{
public:
    LineNumberArea(BaseEditor *editor) : QWidget(editor)
    {
        baseEditor = editor;
    }
    QSize sizeHint() const
    {
        return QSize(baseEditor->lineNumberAreaWidth(), 0);
    }
protected:
    void paintEvent(QPaintEvent *event)
    {
        baseEditor->lineNumberAreaPaintEvent(event);
    }

private:
    BaseEditor *baseEditor;
};

#endif // BASEEDITOR_H
