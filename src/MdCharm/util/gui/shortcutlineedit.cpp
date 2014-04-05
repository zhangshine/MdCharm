#include <QKeyEvent>

#include "shortcutlineedit.h"

ShortcutLineEdit::ShortcutLineEdit(QWidget *parent) :
    QLineEdit(parent)
{
}

void ShortcutLineEdit::keyPressEvent(QKeyEvent *e)
{
    QStringList keys;
    Qt::KeyboardModifiers modifiers = e->modifiers();
    if(modifiers!=Qt::NoModifier){
        if(modifiers.testFlag(Qt::ShiftModifier))
            keys.append("Shift");
        if(modifiers.testFlag(Qt::ControlModifier))
            keys.append("Ctrl");
        if(modifiers.testFlag(Qt::AltModifier))
            keys.append("Alt");
        if(modifiers.testFlag(Qt::MetaModifier))
            keys.append("Meta");
    }
    if(e->key()!=0 && e->key()!=Qt::Key_unknown){
        keys.append(QKeySequence(e->key()).toString());
    }
    setText(keys.join("+"));
    emit textEdited(keys.join("+"));
}
