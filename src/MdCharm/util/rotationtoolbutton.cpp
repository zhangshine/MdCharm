#include "rotationtoolbutton.h"

#include <QStylePainter>
#include <QStyleOptionToolButton>

static const int Vertical_Mask = 0x02;

RotationToolButton::RotationToolButton(QWidget *parent) :
    QToolButton(parent), rot(NoRotation)
{
}

void RotationToolButton::setRotation(Rotation rotation)
{
    if(rot != rotation){
        rot = rotation;
        switch (rotation) {
            case NoRotation:
            case UpsideDown:
                setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
                break;
            case Clockwise:
            case CounterClockwise:
                setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Minimum);
                break;
            default:
                break;
        }
        updateGeometry();
        update();
    }
}

RotationToolButton::Rotation RotationToolButton::getRotation() const
{
    return rot;
}

QSize RotationToolButton::sizeHint() const
{
    QSize size = QToolButton::sizeHint();

    if(rot & Vertical_Mask)
        size.transpose();
    return size;
}

QSize RotationToolButton::minimumSizeHint() const
{
    return sizeHint();
}

void RotationToolButton::paintEvent(QPaintEvent *)
{
    QStylePainter painter(this);
    painter.rotate(rot);
    switch (rot) {
        case UpsideDown:
            painter.translate(-width(), -height());
            break;
        case Clockwise:
            painter.translate(0, -width());
            break;
        case CounterClockwise:
            painter.translate(-height(), 0);
            break;
        default:
            break;
    }

    QStyleOptionToolButton opt;
    initStyleOption(&opt);
    if(rot & Vertical_Mask){
        QSize size = opt.rect.size();
        size.transpose();
        opt.rect.setSize(size);
    }
    painter.drawComplexControl(QStyle::CC_ToolButton, opt);
}
