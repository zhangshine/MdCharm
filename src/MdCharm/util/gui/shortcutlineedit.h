#ifndef SHORTCUTLINEEDIT_H
#define SHORTCUTLINEEDIT_H

#include <QLineEdit>

class ShortcutLineEdit : public QLineEdit
{
    Q_OBJECT
public:
    explicit ShortcutLineEdit(QWidget *parent = 0);

protected:
    virtual void keyPressEvent(QKeyEvent *e);
    
signals:
    
public slots:
    
};

#endif // SHORTCUTLINEEDIT_H
