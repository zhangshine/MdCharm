#include "baseautocompleter.h"

BaseAutoCompleter::BaseAutoCompleter(QObject *parent) :
    QObject(parent)
{
    pairCharMap.insert('[',']');
    pairCharMap.insert('{', '}');
    pairCharMap.insert('\"','\"');
    pairCharMap.insert('\'','\'');
    pairCharMap.insert('(', ')');
    pairCharMap.insert('`', '`');
}

int BaseAutoCompleter::paragraphSeparatorAboutToBeInserted(QTextCursor &cursor)
{
    Q_UNUSED(cursor);
    return 0;
}

QString BaseAutoCompleter::autoComplete(QTextCursor &cursor, const QString &textToInsert) const
{
    if(textToInsert.isEmpty())
        return QString();
    QChar firstChar = textToInsert[0];
    QChar firstCharPairValue = pairCharMap.value(firstChar);
    if(textToInsert.length()==1 && !firstCharPairValue.isNull()){
        if(cursor.hasSelection()){
            return cursor.selectedText() + firstCharPairValue;
        } else {
            return firstCharPairValue;
        }
    }
    return QString();
}
