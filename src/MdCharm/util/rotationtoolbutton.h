#ifndef ROTATIONTOOLBUTTON_H
#define ROTATIONTOOLBUTTON_H

#include <QToolButton>

class RotationToolButton : public QToolButton
{
    Q_OBJECT
public:
    enum Rotation {
        NoRotation = 0,
        UpsideDown = 180,
        Clockwise = 90,
        CounterClockwise = 270
    };

public:
    explicit RotationToolButton(QWidget *parent = 0);
    void setRotation(Rotation rotation);
    Rotation getRotation() const;
    virtual QSize sizeHint() const;
    virtual QSize minimumSizeHint() const;
protected:
    void paintEvent(QPaintEvent *);
private:
    Rotation rot;
signals:
    
public slots:
    
};

#endif // ROTATIONTOOLBUTTON_H
