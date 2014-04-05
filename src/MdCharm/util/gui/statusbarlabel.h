#ifndef STATUSBARLABEL_H
#define STATUSBARLABEL_H

#include <QLabel>

class StatusBarLabel : public QLabel
{
    Q_OBJECT
public:
    StatusBarLabel(QWidget *parent = 0, Qt::WindowFlags f = 0);
protected:
    virtual void mouseReleaseEvent(QMouseEvent *ev);
signals:
    void labelClicked();
};

#endif // STATUSBARLABEL_H
