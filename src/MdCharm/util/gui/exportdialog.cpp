#include <QDir>
#include <QMessageBox>
#include <QTimer>

#include "exportdialog.h"
#include "ui_exportdialog.h"
#include "utils.h"
#include "configuration.h"

ExportDialog::ExportDialog(QWidget *parent) :
    QDialog(parent, Qt::WindowTitleHint|Qt::WindowSystemMenuHint),
    ui(new Ui::ExportDialog)
{
    ui->setupUi(this);

    conf = Configuration::getInstance();
    restoreState();

    connect(ui->htmlCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(htmlCheckBoxActionSlot(bool)));
    connect(ui->htmlPushButton, SIGNAL(clicked()),
            this, SLOT(htmlPushButtonSlot()));
    connect(ui->pdfCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(pdfCheckBoxActionSlot(bool)));
    connect(ui->pdfPushButton, SIGNAL(clicked()),
            this, SLOT(pdfPushButtonSlot()));
    connect(ui->odtCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(odtCheckBoxActionSlot(bool)));
    connect(ui->odtPushButton, SIGNAL(clicked()),
            this, SLOT(odtPushButtonSlot()));
    connect(ui->selectAllCheckBox, SIGNAL(stateChanged(int)),
            this, SLOT(selectAllCheckBoxSlot(int)));
    connect(ui->exportButtonBox, SIGNAL(rejected()),
            this, SLOT(close()));
    connect(ui->exportButtonBox, SIGNAL(accepted()),
            this, SLOT(okButtonSlot()));
}


void ExportDialog::restoreState()
{
    //Load last operation
    int checked = 0;
    QVariant var = conf->getLastStateValue("ExportDialog_HtmlCheckBox");
    if(var.isValid() && var.canConvert(QVariant::Bool) && var.toBool()){
        checked++;
        ui->htmlCheckBox->setChecked(true);
        var = conf->getLastStateValue("ExportDialog_HtmlLineEdit");
        if(var.isValid() && var.canConvert(QVariant::String))
            ui->htmlLineEdit->setText(var.toString());
    } else {
        ui->htmlLineEdit->setEnabled(false);
        ui->htmlPushButton->setEnabled(false);
    }

    var = conf->getLastStateValue("ExportDialog_PdfCheckBox");
    if(var.isValid() && var.canConvert(QVariant::Bool) && var.toBool()){
        checked++;
        ui->pdfCheckBox->setChecked(true);
        var = conf->getLastStateValue("ExportDialog_PdfLineEdit");
        if(var.isValid() && var.canConvert(QVariant::String))
            ui->pdfLineEdit->setText(var.toString());

    } else {
        ui->pdfLineEdit->setEnabled(false);
        ui->pdfPushButton->setEnabled(false);
    }

    var = conf->getLastStateValue("ExportDialog_OdtCheckBox");
    if(var.isValid() && var.canConvert(QVariant::Bool) && var.toBool()){
        checked++;
        ui->odtCheckBox->setChecked(true);
        var = conf->getLastStateValue("ExportDialog_OdtLineEdit");
        if(var.isValid() && var.canConvert(QVariant::String))
            ui->odtLineEdit->setText(var.toString());
    } else {
        ui->odtLineEdit->setEnabled(false);
        ui->odtPushButton->setEnabled(false);
    }

    ui->selectAllCheckBox->setTristate(true);
    if(checked==0)
        ui->selectAllCheckBox->setCheckState(Qt::Unchecked);
    else if(checked==3)
        ui->selectAllCheckBox->setCheckState(Qt::Checked);
    else
        ui->selectAllCheckBox->setCheckState(Qt::PartiallyChecked);
}

ExportDialog::~ExportDialog()
{
    delete ui;
}

void ExportDialog::saveState()
{
    conf->setLastStateValue("ExportDialog_HtmlCheckBox", ui->htmlCheckBox->isChecked());
    conf->setLastStateValue("ExportDialog_HtmlLineEdit", ui->htmlLineEdit->text());
    conf->setLastStateValue("ExportDialog_PdfCheckBox", ui->pdfCheckBox->isChecked());
    conf->setLastStateValue("ExportDialog_PdfLineEdit", ui->pdfLineEdit->text());
    conf->setLastStateValue("ExportDialog_OdtCheckBox", ui->odtCheckBox->isChecked());
    conf->setLastStateValue("ExportDialog_OdtLineEdit", ui->odtLineEdit->text());
}

void ExportDialog::htmlCheckBoxActionSlot(bool b)
{
    ui->htmlLineEdit->setEnabled(b);
    ui->htmlPushButton->setEnabled(b);
    updateSelectAllCheckBoxStatus();
}

void ExportDialog::pdfCheckBoxActionSlot(bool b)
{
    ui->pdfLineEdit->setEnabled(b);
    ui->pdfPushButton->setEnabled(b);
    updateSelectAllCheckBoxStatus();
}

void ExportDialog::odtCheckBoxActionSlot(bool b)
{
    ui->odtLineEdit->setEnabled(b);
    ui->odtPushButton->setEnabled(b);
    updateSelectAllCheckBoxStatus();
}

void ExportDialog::selectAllCheckBoxSlot(int state)
{
    switch(state)
    {
        case Qt::Unchecked:
            ui->htmlCheckBox->setChecked(false);
            ui->pdfCheckBox->setChecked(false);
            ui->odtCheckBox->setChecked(false);
            break;
        case Qt::Checked:
            ui->htmlCheckBox->setChecked(true);
            ui->pdfCheckBox->setChecked(true);
            ui->odtCheckBox->setChecked(true);
            break;
        case Qt::PartiallyChecked:
            break;
        default:
            break;
    }
}

void ExportDialog::exportOneFileFinished(bool isSuccess, bool addPer)
{
    if(addPer){
        ui->exportProgressBar->setValue(ui->exportProgressBar->value()+per);
        if(isSuccess)
            log.append("OK\n");
        else
            log.append("Failed\n");
    }
    if(stack.isEmpty()){
        ui->exportProgressBar->setValue(100);
        QMessageBox::information(this, tr("Export Result"), log);
        close();
    } else {
        ExportType et = stack.pop();
        if(et==HTML){
            log.append(tr("Export to Html file: "));
            emit exportToHtml(ui->htmlLineEdit->text());
        }else if(et==PDF){
            log.append(tr("Export to Pdf file: "));
            emit exportToPdf(ui->pdfLineEdit->text());
        }else if (et==ODT){
            log.append(tr("Export to Odt file: "));
            emit exportToOdt(ui->odtLineEdit->text());
        }else{

        }
    }
}

void ExportDialog::updateSelectAllCheckBoxStatus()
{
    if(ui->htmlCheckBox->isChecked()&&ui->pdfCheckBox->isChecked()&&ui->odtCheckBox->isChecked()){
        ui->selectAllCheckBox->setCheckState(Qt::Checked);
    } else if (ui->htmlCheckBox->isChecked()==false&&ui->pdfCheckBox->isChecked()==false&&ui->odtCheckBox->isChecked()==false){
        ui->selectAllCheckBox->setCheckState(Qt::Unchecked);
    } else{
        ui->selectAllCheckBox->setCheckState(Qt::PartiallyChecked);
    }
}

void ExportDialog::htmlPushButtonSlot()
{
    QString suffix = QString::fromLatin1(".html");
    QString filePath = Utils::getSaveFileName(suffix, this, tr("Save File"),
                                              ui->htmlLineEdit->text(), tr("Html files(*.html)"));
    if(!filePath.isEmpty()){
        ui->htmlLineEdit->setText(filePath);
        updateOtherLineEdits(filePath);
    }
}

void ExportDialog::pdfPushButtonSlot()
{
    QString suffix = QString::fromLatin1(".pdf");
    QString filePath = Utils::getSaveFileName(suffix, this, tr("Save File"),
                                              ui->pdfLineEdit->text(), tr("Pdf files(*.pdf)"));
    if(!filePath.isEmpty()){
        ui->pdfLineEdit->setText(filePath);
        updateOtherLineEdits(filePath);
    }
}

void ExportDialog::odtPushButtonSlot()
{
    QString suffix = QString::fromLatin1(".odt");//multi suffixs
    QString filePath = Utils::getSaveFileName(suffix, this, tr("Save File"),
                                              ui->odtLineEdit->text(), tr("ODT files(*.odt)"));
    if(!filePath.isEmpty()){
        ui->odtLineEdit->setText(filePath);
        updateOtherLineEdits(filePath);
    }
}

void ExportDialog::updateOtherLineEdits(const QString &filePath)
{
    QFileInfo fileInfo(filePath);
    QString filePathWithoutSuffix = filePath.left(filePath.length()-fileInfo.suffix().length());
    if(ui->htmlLineEdit->text().isEmpty())
        ui->htmlLineEdit->setText(filePathWithoutSuffix+QString::fromLatin1("html"));
    if(ui->pdfLineEdit->text().isEmpty())
        ui->pdfLineEdit->setText(filePathWithoutSuffix+QString::fromLatin1("pdf"));
    if(ui->odtLineEdit->text().isEmpty())
        ui->odtLineEdit->setText(filePathWithoutSuffix+QString::fromLatin1("odt"));
}

void ExportDialog::okButtonSlot()
{
    if(ui->selectAllCheckBox->checkState()==Qt::Unchecked)
        return;
    //assert line edit is not empty
    if(ui->htmlCheckBox->isChecked() &&
            (ui->htmlLineEdit->text().isEmpty()||
             ui->htmlLineEdit->text().trimmed().isEmpty())){
        QMessageBox::warning(this, tr("Warning"), tr("Html file path can't be empty!"));
        return;
    }
    if(ui->odtCheckBox->isChecked() &&
            (ui->odtLineEdit->text().isEmpty()||
             ui->odtLineEdit->text().trimmed().isEmpty())){
        QMessageBox::warning(this, tr("Warning"), tr("Odt file path can't be empty!"));
        return;
    }
    if(ui->pdfCheckBox->isChecked() &&
            (ui->pdfLineEdit->text().isEmpty()||
             ui->pdfLineEdit->text().trimmed().isEmpty())){
        QMessageBox::warning(this, tr("Warning"), tr("Pdf file path can't be empty!"));
        return;
    }

    ui->selectAllCheckBox->setEnabled(false);
    ui->htmlCheckBox->setEnabled(false);
    ui->htmlLineEdit->setEnabled(false);
    ui->htmlPushButton->setEnabled(false);
    ui->pdfCheckBox->setEnabled(false);
    ui->pdfLineEdit->setEnabled(false);
    ui->pdfPushButton->setEnabled(false);
    ui->odtCheckBox->setEnabled(false);
    ui->odtLineEdit->setEnabled(false);
    ui->odtPushButton->setEnabled(false);
    ui->exportButtonBox->setEnabled(false);
    int i=0;
    stack.clear();
    if(ui->htmlCheckBox->isChecked()){
        i++;
        stack.push(HTML);
    }
    if(ui->pdfCheckBox->isChecked()){
        i++;
        stack.push(PDF);
    }
    if(ui->odtCheckBox->isChecked()){
        i++;
        stack.push(ODT);
    }
    if(i==0){
        close();
        return;
    }
    per = 100 / i;
    saveState();
    QTimer::singleShot(50, this, SLOT(startExport()));
}

void ExportDialog::startExport()
{
    exportOneFileFinished(false, false);
}
