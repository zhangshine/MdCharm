#ifndef BASEAUTOCOMPLETER_H
#define BASEAUTOCOMPLETER_H

#include <QObject>
#include <QTextCursor>

class BaseAutoCompleter : public QObject
{
    Q_OBJECT
public:
    explicit BaseAutoCompleter(QObject *parent = 0);
    virtual int paragraphSeparatorAboutToBeInserted(QTextCursor &cursor);
    // Returns the text to complete at the cursor position, or an empty string
    virtual QString autoComplete(QTextCursor &cursor, const QString &text) const;
    
signals:
    
public slots:

protected:
private:
    QMap<QChar, QChar> pairCharMap;
};

#endif // BASEAUTOCOMPLETER_H
