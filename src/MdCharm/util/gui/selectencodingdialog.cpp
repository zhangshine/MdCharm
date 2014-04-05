#include "selectencodingdialog.h"
#include "ui_selectencodingdialog.h"
#include "../../utils.h"
#include "../../configuration.h"

#include <QtGui>

#ifdef QT_V5
#include <QtWidgets>
#endif

SelectEncodingDialog::SelectEncodingDialog(bool modified, QString currentEncoding, QWidget *parent) :
    QDialog(parent, Qt::WindowTitleHint|Qt::WindowSystemMenuHint),
    ui(new Ui::SelectEncodingDialog)
{
    ui->setupUi(this);
    this->modified = modified;
    conf = Configuration::getInstance();
    encodingComboBox = ui->encodingComboBox;
    reloadFileCheckBox = ui->reloadFileCheckBox;
    buttonBox = ui->buttonBox;
    connect(buttonBox, SIGNAL(accepted()),
            this, SLOT(reloadFileWarning()));
    connect(buttonBox, SIGNAL(rejected()),
            this, SLOT(reject()));
    QStringList sl = Utils::getEncodingList();
    int index = sl.indexOf(currentEncoding);
    encodingComboBox->addItems(sl);
    encodingComboBox->setCurrentIndex(index);
    reloadFileCheckBox->setChecked(true);
}

SelectEncodingDialog::~SelectEncodingDialog()
{
    delete ui;
}

QString SelectEncodingDialog::getSelectedEncoding()
{
    return encodingComboBox->currentText();
}

bool SelectEncodingDialog::isReloadFile()
{
    return reloadFileCheckBox->isChecked();
}

void SelectEncodingDialog::reloadFileWarning()
{
    if(isReloadFile()&&modified)
    {
        if(QMessageBox::No == QMessageBox::warning(this, tr("Reload File"),
                                tr("File changes will lost if reload this file."
                                   " Do you really want to reload this file?"),
                                   QMessageBox::Yes|QMessageBox::No))
        {
            return;
        }
    }
    accept();
}
