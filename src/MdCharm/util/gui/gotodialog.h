#ifndef GOTODIALOG_H
#define GOTODIALOG_H

#include <QDialog>

namespace Ui {
class GotoDialog;
}

class GotoDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit GotoDialog(int max, int min=1, QWidget *parent = 0);
    ~GotoDialog();
    int getLineNumber();
private slots:
    void checkLineNumber();
private:
    Ui::GotoDialog *ui;
    int max;
    int min;
    int togo;
};

#endif // GOTODIALOG_H
