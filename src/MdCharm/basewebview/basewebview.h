#ifndef BASEWEBVIEW_H
#define BASEWEBVIEW_H

#include <QWebView>

class BaseWebView : public QWebView
{
    Q_OBJECT
public:
    explicit BaseWebView(QWidget *parent = 0);
protected:
    virtual void contextMenuEvent(QContextMenuEvent *e);
signals:
    
public slots:
};

#endif // BASEWEBVIEW_H
