#ifndef MARKDOWNEDITAREAWIDGET_H
#define MARKDOWNEDITAREAWIDGET_H

#include "editareawidget.h"

#include <QUrl>
#include <QTextDocument>

#ifdef QT_V5
#include <QtPrintSupport>
#endif

class QSplitter;
class QPlainTextEdit;
class MarkdownWebView;
class BaseEditor;
class MarkdownEditor;
class FindAndReplace;
class QScrollBar;
class HighLighter;
class MdCharmForm;

//This class is useless
class MarkdownWebkitHandler : public QObject
{
    Q_OBJECT
public:
    MarkdownWebkitHandler();
    ~MarkdownWebkitHandler();
    bool isDomReady();
signals:
    void updateContent(const QString&);
    void initContent();
public slots:
    void domReady();
private:
    bool m_domReady;
};

class MarkdownEditAreaWidget : public EditAreaWidget
{
    Q_OBJECT
public:
    explicit MarkdownEditAreaWidget(MdCharmForm *mainForm, const QString &filePath=QString(),
                                    const QUrl &baseUrl=QUrl());
    ~MarkdownEditAreaWidget();
    virtual void setText(const QString &text);
    virtual QString getText();
    virtual void exportToPdf(const QString &filePath);
    virtual void exportToODT(const QString &filePath);
    virtual void exportToHtml(const QString &filePath);
    virtual void printContent();
    virtual void printPreview();
    virtual bool saveFile();
    virtual void saveFileAs();
    virtual void switchPreview(int type);
    virtual void changeSyncScrollbarSetting(bool sync);
    virtual void reloadFile();
    virtual int getCurrentMaxBlockCount();
    virtual void gotoLine(int line);
    virtual const StateModel getState();
    virtual void restoreFileState(const StateModel &sm);
    void showFind();
    void dealMarkdownMenuAction(int type);
    void disableSpellCheck();
    void enableSpellCheck();
    QTextDocument* document();

    void jumpToPreviewAnchor(const QString &anchor);

private:
    explicit MarkdownEditAreaWidget(MarkdownEditAreaWidget &src);
    void initPreviewerMatter();
    void initHtmlEngine();
    void initGui();
    void initConfiguration();
    void initContent(const QString &filePath);
    void initSignalsAndSlots();
    void insertLinkOrPicture(int type);
    void insertCode();
    std::string convertMarkdownToHtml();
public:
    virtual EditAreaWidget* clone();
    QString getProDir();
private:
    MdCharmForm *mainForm;
    QSplitter *splitter;
    QScrollBar *editorScrollBar;
    MarkdownEditor *editor;
    MarkdownWebView *previewer;
    MarkdownWebkitHandler *markdownWebkitHandler;
    HighLighter *highlighter;
    FindAndReplace *findAndReplaceWidget;
    QSharedPointer<QTextDocument> doc;
//    int lastRevision;

    bool inited;
    QUrl baseUrl;
protected:
    virtual void resizeEvent(QResizeEvent *event);
    
signals:
    void addToRecentFileList(const QString &path);
    void focusInSignal();
    void textChanged();
    
public slots:
    void parseMarkdown();
    void reFind();
    void addJavascriptObject();
    virtual void copy();
    virtual void cut();
    virtual void paste();
    virtual void redo();
    virtual void undo();
    virtual void selectAll();
    virtual bool isModified();
    virtual bool isUndoAvailable();
    virtual bool isRedoAvailable();
    virtual void setModified(bool isModi);
    virtual void findNext();
    virtual void findPrevious();

    virtual void updateConfiguration();
    void setFocusEditor();
    void setFocusFinder();
    void hideFind();
private slots:
    void cursorPositionChanged();
    void overWriteModeChanged();
    void scrollPreviewTo(int value);
    void scrollPreviewTo();
    void openUrl(const QUrl &url);
    void paintRequestSlot(QPrinter *printer);
};

#endif // MARKDOWNEDITAREAWIDGET_H
