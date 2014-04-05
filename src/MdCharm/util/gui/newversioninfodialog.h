#ifndef NEWVERSIONINFODIALOG_H
#define NEWVERSIONINFODIALOG_H

#include <QDialog>
#include <QGridLayout>
#include <QTextBrowser>
#include <QDialogButtonBox>
#include <QPushButton>

#include "configuration.h"

class NewVersionInfoDialog : public QDialog
{
    Q_OBJECT
public:
    explicit NewVersionInfoDialog(const QString &version, const QString &revision, const QString &infoMarkdown, QWidget *parent = 0);
    
signals:
    
public slots:
private slots:
    void buttonClicked(QAbstractButton *btn);
private:
    QTextBrowser *infoBrower;
    QDialogButtonBox *buttonBox;
    QPushButton *remindMeLaterPushButton;
    QPushButton *ignoreThisVersionButton;
    QPushButton *downloadPushButton;

    Configuration *conf;
    QString rev;
};

#endif // NEWVERSIONINFODIALOG_H
