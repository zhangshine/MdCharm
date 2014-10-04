// Copyright (c) 2014 zhangshine. All rights reserved.
// Use of this source code is governed by a BSD license that can be
// found in the LICENSE file.

#include "tocdockwidget.h"
#include "ui_tocdockwidget.h"
#include "configuration.h"
#include "util/updatetocthread.h"

#include <QtWebKit>

TOCDockWidget::TOCDockWidget(QWidget *parent) :
    QDockWidget(parent),
    ui(new Ui::TOCDockWidget)
{
    ui->setupUi(this);

    ui->webView->page()->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);

    thread = new UpdateTocThread(this);

    QFile htmlTemplate(":/markdown/toc.html");
    if(htmlTemplate.open(QIODevice::ReadOnly))
    {
        QString htmlContent = htmlTemplate.readAll();
        ui->webView->setHtml(htmlContent);
        htmlTemplate.close();
    }

    connect(this, SIGNAL(visibilityChanged(bool)), this, SLOT(visibleChange(bool)));
    connect(thread, SIGNAL(finished()), this, SLOT(workerFinished()));
    connect(thread, SIGNAL(workerResult(QString)), this, SLOT(updateTocContent(QString)));
    connect(ui->webView, SIGNAL(linkClicked(QUrl)), this, SIGNAL(anchorClicked(QUrl)));
}

void TOCDockWidget::visibleChange(bool b)
{
    Configuration *conf = Configuration::getInstance();
    conf->setTocDockWidgetVisible(b);
}

void TOCDockWidget::workerFinished()
{
    mutex.lock();

    if(!this->content.isEmpty() && thread->isFinished()){
        thread->setContent(this->type, this->content);
        thread->start();
        this->content.clear();
    }

    mutex.unlock();
}

TOCDockWidget::~TOCDockWidget()
{
    delete ui;
    thread->quit();
    delete thread;
}

void TOCDockWidget::updateToc(MarkdownToHtml::MarkdownType type, QString &content)
{
    mutex.lock();

    if(!thread->isRunning()){
        thread->setContent(type, content);
        thread->start();
        this->content.clear();
    } else {
        this->type = type;
        this->content = content;
    }

    mutex.unlock();
}

void TOCDockWidget::updateTocContent(const QString &result)
{
    ui->webView->page()->currentFrame()->findFirstElement("body").setInnerXml(result);
}
