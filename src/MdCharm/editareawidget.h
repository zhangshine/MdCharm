#ifndef EDITAREAWIDGET_H
#define EDITAREAWIDGET_H

#include <QWidget>

class Configuration;

class FileModel
{
private:
    bool hasBom;
    QString fileFullPath;
    QString encodingFormatName;
public:
    FileModel();
    void setFileFullPath(const QString filePath);
    void setHasBom(bool b);
    void setEncodingFormatName(const QString c);
    QString getEncodingFormatName();

    bool isHasBom();
    QString getFileFullPath();
    QString getFileName();
    bool isUntitled();
};

class EditorModel
{
public:
    enum EditorType
    {
        NONE,
        BROWER,
        EDITABLE,//seperator
        MARKDOWN
    };

    EditorModel()
    {
        currentLineNumber = 1;
        currentColumnNumber = 1;
        overWrite = false;
        canCopy = false;
        findVisible = false;
        editorType = NONE;
    }

private:
    int currentLineNumber;
    int currentColumnNumber;
    bool overWrite;
    bool canCopy;
    bool findVisible;
    EditorType editorType;
public:
    void setFindVisible(bool f);
    bool isFindVisible();
    void setCanCopy(bool c);
    bool isCopyAvailable();
    void setCurrentLineNumber(int l);
    int getCurrentLineNumber();
    void setCurrentColumnNumber(int c);
    int getCurrentColumnNumber();
    void overWriteChanged();
    bool isOverWrite();
    void setEditorType(EditorType t);
    EditorType getEditorType();
    bool isEditable();
};

class StateModel
{
public:
    bool isValid();
    bool isCurrentTab();
    void setIsCurrentTab(bool b);
    int getSelectionStart()const;
    void setSelectionStart(int start);
    int getSelectionEnd()const;
    void setSelectionEnd(int end);
    int getFirstVisibleLine()const;
    void setFirstVisibleLine(int line);
    const QString getFileFullPath()const;
    void setFileFullPath(const QString &ffp);
    int getVerticalScorllBarCurrentValue()const;
    void setVerticalScrllBarCurrentValue(int value);
    int getVerticalScrollBarMaxValue()const;
    void setVerticalScrollBarMaxValue(int value);
    int getViewNum();
    void setViewNum(int v);
private:
    bool currentTab;
    int selectionStart;
    int selectionEnd;
    int firstVisibleLine;
    int verticalScrollBarCurrentValue;
    int verticalScrollBarMaxValue;
    int viewNum;
    QString fileFullPath;
};

class EditAreaWidget : public QWidget
{
    Q_OBJECT
public:
    enum EditActionOption{
        AllowSelectAll = 1 << 0,
        AllowSaveAs = 1 << 1,
        AllowExportToHtml = 1 << 2,
        AllowExportToPdf = 1 << 3,
        AllowExportToODT = 1 << 4,
        AllowPrint = 1 << 5,
        AllowPreview = 1 << 6,
        AllowFind = 1 << 7,
        AllowSplit = 1 << 8
    };
    Q_DECLARE_FLAGS(EditActionOptions, EditActionOption)
private:
public:
    explicit EditAreaWidget(const QString &filePath=QString(), EditActionOptions op=0);
    explicit EditAreaWidget(EditAreaWidget &e);
    virtual void setText(const QString &text);
    virtual QString getText();
    virtual void exportToPdf(const QString &filePath);
    virtual void exportToODT(const QString &filePath);
    virtual void exportToHtml(const QString &filePath);
    virtual void printContent();
    virtual void printPreview();
    virtual bool saveFile();
    virtual void saveFileAs();
    virtual void switchPreview(int isShowPreview);
    virtual void changeSyncScrollbarSetting(bool sync);
    virtual void reloadFile();
    virtual int getCurrentMaxBlockCount();
    virtual void gotoLine(int line);
    virtual const StateModel getState();
    virtual void restoreFileState(const StateModel &sm);
    virtual void changeFilePath(const QString &newFilePath);
    virtual EditAreaWidget* clone() = 0;
    FileModel getFileModel();
    EditorModel getEditorModel();
    bool isEditActionOptionEnabled(EditActionOption op);
protected:
    QSharedPointer<FileModel> fm;
    EditorModel em;
    Configuration *conf;
    EditActionOptions options;
signals:
    void editorContentModifiedSignal(bool b);
    void updateActions();
    void updateStatusBar();
    void showStatusMessage(const QString &msg);
public slots:
    virtual void copy() = 0;
    virtual void cut() = 0;
    virtual void paste() = 0;
    virtual void redo() = 0;
    virtual void undo() = 0;
    virtual void selectAll() = 0;
    virtual bool isModified() = 0;
    virtual void setModified(bool isModi) = 0;
    virtual bool isUndoAvailable() = 0;
    virtual bool isRedoAvailable() = 0;
    virtual void findNext();
    virtual void findPrevious();
    void setCopyAvaliable(bool a);
    void setFileEncoding(const QString en);
    virtual void updateConfiguration();
};

Q_DECLARE_OPERATORS_FOR_FLAGS(EditAreaWidget::EditActionOptions)

#endif // EDITAREAWIDGET_H
