#include <QFileInfo>
#include <QMessageBox>

#include "renamefiledialog.h"
#include "ui_renamefiledialog.h"
#include "utils.h"

RenameFileDialog::RenameFileDialog(const QString &fileFullPath, QWidget *parent) :
    QDialog(parent, Qt::WindowTitleHint|Qt::WindowSystemMenuHint),
    ui(new Ui::RenameFileDialog)
{
    ui->setupUi(this);
    oldFilePath = fileFullPath;
    QString filename = QFileInfo(fileFullPath).fileName();
    ui->infoLabel->setText(tr("Rename file %1 to:").arg(filename));
    ui->renameLineEdit->setText(filename);
    connect(ui->buttonBox, SIGNAL(rejected()), this, SLOT(close()));
    connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(acceptSlot()));
}

RenameFileDialog::~RenameFileDialog()
{
    delete ui;
}

QString RenameFileDialog::getNewFilePath()
{
    return newFP;
}

void RenameFileDialog::acceptSlot()
{
    if(ui->renameLineEdit->text().isEmpty() ||
            ui->renameLineEdit->text().trimmed().isEmpty()){
        QMessageBox::warning(this, tr("File name can't be empty!"), tr("File name can't be empty!"));
        return;
    }
    QFileInfo fi(oldFilePath);
    QString filename = fi.fileName();
    QString suffix = fi.suffix();
    QString newFilePath = oldFilePath.left(oldFilePath.length()-filename.length())+ui->renameLineEdit->text();
    if(!Utils::isMarkdownFile(newFilePath)){//TODO: currently only support markdown suffix
        newFilePath.append(".");
        newFilePath.append(suffix);
    }
    if(QFile::exists(newFilePath)){
        QMessageBox::warning(this, tr("This file is already exists!"), tr("This file is already exists!"));
        return;
    }
    newFP = newFilePath;
    accept();
}
