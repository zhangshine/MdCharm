#ifndef MARKDOWNWEBVIEW_H
#define MARKDOWNWEBVIEW_H

#include "basewebview.h"

class MarkdownWebView : public BaseWebView
{
    Q_OBJECT
public:
    explicit MarkdownWebView(QWidget *parent = 0);
    
signals:
    
public slots:
    void reload();
};

#endif // MARKDOWNWEBVIEW_H
