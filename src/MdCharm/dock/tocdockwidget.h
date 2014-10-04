// Copyright (c) 2014 zhangshine. All rights reserved.
// Use of this source code is governed by a BSD license that can be
// found in the LICENSE file.

#ifndef TOCDOCKWIDGET_H
#define TOCDOCKWIDGET_H

#include <QDockWidget>
#include <QMutex>
#include <QUrl>

#include "markdowntohtml.h"

class UpdateTocThread;

namespace Ui {
class TOCDockWidget;
}

class TOCDockWidget : public QDockWidget
{
    Q_OBJECT

public:
    explicit TOCDockWidget(QWidget *parent = 0);
    ~TOCDockWidget();

public slots:
    void updateToc(MarkdownToHtml::MarkdownType type, QString &content);

signals:
    void anchorClicked(const QUrl &link);

private slots:
    void visibleChange(bool b);
    void workerFinished();
    void updateTocContent(const QString &result);

private:
    Ui::TOCDockWidget *ui;

    QMutex mutex;
    MarkdownToHtml::MarkdownType type;
    QString content;
    UpdateTocThread *thread;
};

#endif // TOCDOCKWIDGET_H
