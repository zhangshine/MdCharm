// Copyright (c) 2014 zhangshine. All rights reserved.
// Use of this source code is governed by a BSD license that can be
// found in the LICENSE file.

#ifndef UPDATETOCTHREAD_H
#define UPDATETOCTHREAD_H

#include <QThread>

#include "markdowntohtml.h"

class EditAreaTabWidgetManager;

class UpdateTocThread : public QThread
{
    Q_OBJECT
public:
    explicit UpdateTocThread( QObject *parent = 0);
    void setContent(MarkdownToHtml::MarkdownType type, const QString &content);

signals:
    void workerResult(const QString result);

public slots:
    void run();

private:
    MarkdownToHtml::MarkdownType type;
    QString content;

};

#endif // UPDATETOCTHREAD_H
