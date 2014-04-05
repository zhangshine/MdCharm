#include <QTextBlock>
#include <QRegExp>

#include "markdownautocompleter.h"
#include "configuration.h"

MarkdownAutoCompleter::MarkdownAutoCompleter(QObject *parent) :
    BaseAutoCompleter(parent)
{
    orderRe = new QRegExp("([0-9]*)\\.\\s");
    conf = Configuration::getInstance();
}

MarkdownAutoCompleter::~MarkdownAutoCompleter()
{
    delete orderRe;
    orderRe = 0;
}

int MarkdownAutoCompleter::paragraphSeparatorAboutToBeInserted(QTextCursor &cursor)
{
    if(!conf->isAutoIndentation())//This method only deal auto-indent currently, so if not enabled, just return
        return 0;
    QTextBlock curBlock = cursor.block();
    if(!curBlock.isValid())
        return 0;
    QString blockText = curBlock.text();
    if(blockText.isEmpty())
        return 0;
    int whitespaces = 0;
    while(whitespaces<blockText.length()){
        QChar c=blockText[whitespaces];
        if(c==QChar(' ')||c==QChar('\t'))
            whitespaces++;
        else
            break;
    }
    if(whitespaces==blockText.length())
        return 0;
    int len = 0;
    if( (whitespaces+1)<blockText.length()){
        if(isUnorderList(blockText[whitespaces], blockText[whitespaces+1])){
            whitespaces += 2;
        } else{
            len = isOrderList(blockText, whitespaces);
        }
    }
    bool leftIsWhitespace = true;
    if(whitespaces==3)
        leftIsWhitespace = false;
    int start = len!=0 ? whitespaces+1+QString::number(len).length() : whitespaces;
    while(start<blockText.length()){
        if(blockText[start]!=QChar(' ')){
            leftIsWhitespace = false;
            break;
        }
        start++;
    }
    if(leftIsWhitespace){//end of list remove the suffix like * , - , + , 1. and so on
        cursor.movePosition(QTextCursor::StartOfBlock);
        cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
        cursor.removeSelectedText();
        cursor.insertBlock();
        return 1;
    }
    cursor.insertBlock();
    QString textToInsert = blockText.left(whitespaces);
    if(len!=0){
        textToInsert.append(QString::number(len+1)).append(". ");
    }
    cursor.insertText(textToInsert);
    return 1;
}

int MarkdownAutoCompleter::isOrderList(QString &text, int start)
{
    int index = orderRe->indexIn(text, start);
    if(index!=start)
        return 0;
    return orderRe->cap(1).toInt();
}

bool MarkdownAutoCompleter::isUnorderList(QChar c1, QChar c2)
{
    return ( c1==QChar('*') || c1==QChar('+') || c1==QChar('-') ) && (c2==QChar(' ') || c2==QChar('\t'));
}
