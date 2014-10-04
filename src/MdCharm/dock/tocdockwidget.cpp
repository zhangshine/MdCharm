// Copyright (c) 2014 zhangshine. All rights reserved.
// Use of this source code is governed by a BSD license that can be
// found in the LICENSE file.

#include "tocdockwidget.h"
#include "ui_tocdockwidget.h"

TOCDockWidget::TOCDockWidget(QWidget *parent) :
    QDockWidget(parent),
    ui(new Ui::TOCDockWidget)
{
    ui->setupUi(this);
}

TOCDockWidget::~TOCDockWidget()
{
    delete ui;
}
