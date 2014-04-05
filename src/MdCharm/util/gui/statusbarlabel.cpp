#include "statusbarlabel.h"
#include <QtGui>

StatusBarLabel::StatusBarLabel(QWidget *parent, Qt::WindowFlags f)
    : QLabel(parent, f)
{
}

void StatusBarLabel::mouseReleaseEvent(QMouseEvent *ev)
{
    ev->accept();
    emit labelClicked();
}
