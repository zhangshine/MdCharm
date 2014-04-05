#include "gotodialog.h"
#include "ui_gotodialog.h"

#include <QMessageBox>

GotoDialog::GotoDialog(int max, int min, QWidget *parent) :
    QDialog(parent, Qt::WindowTitleHint|Qt::WindowSystemMenuHint),
    ui(new Ui::GotoDialog)
{
    ui->setupUi(this);
    this->max = max;
    this->min = min;
    ui->gotoLabel->setText(tr("Line(%1 - %2)").arg(min).arg(max));
    connect(ui->buttonBox, SIGNAL(rejected()),
            this, SLOT(reject()));
    connect(ui->buttonBox, SIGNAL(accepted()),
            this, SLOT(checkLineNumber()));
}

void GotoDialog::checkLineNumber()
{
    QString text = ui->gotoLineEdit->text();
    togo = text.toInt();
    if(togo>max || togo<min)
    {
        QMessageBox::warning(this, tr("Line number not valid"),
                             tr("Line number must between %1 and %2.").arg(min).arg(max));
        return;
    }
    //else
    accept();
}

int GotoDialog::getLineNumber()
{
    return togo;
}

GotoDialog::~GotoDialog()
{
    delete ui;
}
