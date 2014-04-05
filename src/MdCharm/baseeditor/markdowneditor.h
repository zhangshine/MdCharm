#ifndef MARKDOWNEDITOR_H
#define MARKDOWNEDITOR_H

#include "baseeditor.h"
#include "utils.h"

class QAction;
class MarkdownAutoCompleter;

class MarkdownEditor : public BaseEditor
{
    Q_OBJECT

public:
    MarkdownEditor(QWidget *parent=0);

protected:
    virtual void keyPressEvent(QKeyEvent *event);
    virtual void contextMenuEvent(QContextMenuEvent *e);
public:
    void italicText();
    void boldText();
    void tabText();
    void untabText();
    void quoteText();
    void strikeThroughText();
    void insertLinkOrPicuture(int type, const QString &text, const QString &url,
                              const QString &title, const QString &width, const QString &height);
    void insertCode(const QString &lan);
private:
    void replaceTextInCurrentCursor(const QString &text);
private slots:
    void copyAsHtmlSlot();
private:
    QList<QAction *> addSpellCheckActions(QMenu *menu);
    QList<QAction *> addSpellCheckLanguageActions(QMenu *menu);
private:
    MdCharmGlobal *mdcharmGlobalInstance;
    QAction *copyAsHtmlAction;
    MarkdownAutoCompleter *autoCompleter;
};

#endif // MARKDOWNEDITOR_H
