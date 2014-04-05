#ifndef BROWEREDITAREAWIDGET_H
#define BROWEREDITAREAWIDGET_H

#include <QUrl>

#include "editareawidget.h"

class BaseWebView;

class BrowerWebkitHandler : public QObject
{
    Q_OBJECT
public:
    BrowerWebkitHandler();
    ~BrowerWebkitHandler();
signals:
    void updateRecentFileList(const QString list);
    void openRecentFile(const QString filePath);
public slots:
    void domReady();
    void updateRecentFiles();
private:
    Configuration *conf;
};

class BrowerEditAreaWidget : public EditAreaWidget
{
    Q_OBJECT
public:
    BrowerEditAreaWidget(const QString &filePath=QString());
    ~BrowerEditAreaWidget();
    void setHtml(const QString &html, const QUrl &baseUrl= QUrl());
    void setUrl(const QUrl &url);
    virtual void setText(const QString &text);
    BrowerWebkitHandler* getWebkitHanlder(){return webkitHandler;}
private:
    void initContent(const QString &filePath);
    void initSignalsAndSlots();
public:
    virtual EditAreaWidget* clone();
private:
    BaseWebView *brower;
    BrowerWebkitHandler *webkitHandler;
protected:
    virtual void resizeEvent(QResizeEvent *event);

public slots:
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

    void addJavascriptObject();
private slots:
    void openLinkOutside(const QUrl &url);
};

#endif // BROWEREDITAREAWIDGET_H
