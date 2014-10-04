// Copyright (c) 2014 zhangshine. All rights reserved.
// Use of this source code is governed by a BSD license that can be
// found in the LICENSE file.

#ifndef TOCDOCKWIDGET_H
#define TOCDOCKWIDGET_H

#include <QDockWidget>

namespace Ui {
class TOCDockWidget;
}

class TOCDockWidget : public QDockWidget
{
    Q_OBJECT

public:
    explicit TOCDockWidget(QWidget *parent = 0);
    ~TOCDockWidget();

private slots:
    void visibleChange(bool b);

private:
    Ui::TOCDockWidget *ui;
};

#endif // TOCDOCKWIDGET_H
