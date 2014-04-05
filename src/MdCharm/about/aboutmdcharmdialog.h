#ifndef ABOUTMDCHARMDIALOG_H
#define ABOUTMDCHARMDIALOG_H

#include <QDialog>

class QPlainTextEdit;
class QLabel;

namespace Ui {
    class AboutMdCharmDialog;
}

class AboutMdCharmDialog : public QDialog
{
    Q_OBJECT

public:
    AboutMdCharmDialog(QWidget *parent);
    ~AboutMdCharmDialog();
private:
    Ui::AboutMdCharmDialog *ui;
    QPushButton *closePushButton;
    QLabel *versionLabel;
};

#endif // ABOUTMDCHARMDIALOG_H
