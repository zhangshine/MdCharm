// Copyright (c) 2014 zhangshine. All rights reserved.
// Use of this source code is governed by a BSD license that can be
// found in the LICENSE file.

#include "updatetocthread.h"

UpdateTocThread::UpdateTocThread(QObject *parent) :
    QThread(parent)
{
}

void UpdateTocThread::run()
{
    if(this->type == MarkdownToHtml::MultiMarkdown){
        emit workerResult(QString());
    } else {
        std::string stdResult;
        QByteArray content = this->content.toUtf8();
        MarkdownToHtml::renderMarkdownExtarToc(this->type, content.data(), content.length(), stdResult);
        emit workerResult(QString::fromUtf8(stdResult.c_str(), stdResult.length()));
    }
}

void UpdateTocThread::setContent(MarkdownToHtml::MarkdownType type, const QString &content)
{
    this->type = type;
    this->content = content;
}
