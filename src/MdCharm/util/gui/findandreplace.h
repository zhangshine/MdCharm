#ifndef FINDANDREPLACE_H
#define FINDANDREPLACE_H

#include <QWidget>
#include <QTextDocument>

class QLineEdit;
class QToolButton;

namespace Ui {
    class FindAndReplaceForm;
}

class FindAndReplace : public QWidget
{
    Q_OBJECT
public:
    explicit FindAndReplace(QWidget *parent = 0);
    ~FindAndReplace();
    void setFocusTextEdit();
    void resendFindTextSignal();
    void resendFindNextSignal();
    void resendFindPreviousSignal();
    void setFindText(const QString &txt);
    
signals:
    void findPrevious(const QString &ft, QTextDocument::FindFlags qff, bool isRE);
    void findNext(const QString &ft, QTextDocument::FindFlags qff, bool isRE);
    void findText(const QString &ft, QTextDocument::FindFlags qff, bool isRE, bool isSetCursor=true);
    void replace(const QString &rt);
    void replaceAll(const QString &ft, const QString &rt, QTextDocument::FindFlags qff, bool isRE);
    void findHide();
private:
    Ui::FindAndReplaceForm *ui;
    QToolButton *closeFindButton;
    QToolButton *prevFindButton;
    QToolButton *nextFindButton;
    QLineEdit *findTextLineEdit;

private slots:
    void regularExpressionCheckedSlot(bool b);
    void find(const QString &ft, bool isSetCursor=true);
    void replaceButtonSlot();
    void replaceAllButtonSlot();
    
public slots:
    void hideFind();
    void previous();
    void next();
};

#endif // FINDANDREPLACE_H
