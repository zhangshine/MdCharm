#include <QMessageBox>
#include <QFileDialog>
#include <QDir>

#include "insertlinkorpicturedialog.h"
#include "ui_insertlinkorpicturedialog.h"
#include "utils.h"
#include "configuration.h"
#include "markdowntohtml.h"

InsertLinkOrPictureDialog::InsertLinkOrPictureDialog(int type, QString proDir, QWidget *parent) :
    QDialog(parent, Qt::WindowTitleHint|Qt::WindowSystemMenuHint),
    ui(new Ui::InsertLinkOrPictureDialog),
    proDir(proDir)
{
    ui->setupUi(this);
    ui->urlLineEdit->setFocus();
    conf = Configuration::getInstance();
    if(type==MdCharmGlobal::ShortcutInsertPicture){
        ui->textLabel->setText(tr("Alt Text:"));
        ui->urlLabel->setText(tr("Picture Url:"));
        if(conf->getMarkdownEngineType()!=MarkdownToHtml::MultiMarkdown){
            ui->widthLineEdit->hide();
            ui->widthLabel->hide();
            ui->heightLineEdit->hide();
            ui->heightLabel->hide();
        }
        setWindowTitle(tr("Insert Picture"));
    } else {
        ui->insertLocalPushButton->hide();
        ui->textLabel->setText(tr("Link Text:"));
        ui->urlLabel->setText(tr("Link Url:"));
        ui->widthLineEdit->hide();
        ui->widthLabel->hide();
        ui->heightLineEdit->hide();
        ui->heightLabel->hide();
        setWindowTitle(tr("Insert Link"));
    }
    connect(ui->buttonBox, SIGNAL(rejected()), this, SLOT(close()));
    connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(checkAndAccept()));
    connect(ui->insertLocalPushButton, SIGNAL(clicked()), this, SLOT(insertLocalFileSlot()));
    this->type = type;
    setFixedHeight(sizeHint().height());
}

InsertLinkOrPictureDialog::~InsertLinkOrPictureDialog()
{
    delete ui;
}

void InsertLinkOrPictureDialog::checkAndAccept()
{
    QString text = ui->textLineEdit->text();
    if(text.isEmpty() && text.trimmed().isEmpty()){
        showWarning(tr("Alt Text can't be empty!"), tr("Link Text cann't be empty!"));
        return;
    }
    QString url = ui->urlLineEdit->text();
    if(url.isEmpty() && url.trimmed().isEmpty()){
        showWarning(tr("Picture Url can't be empty!"), tr("Link Url cann't be empty!"));
        return;
    }
    accept();
}

void InsertLinkOrPictureDialog::showWarning(const QString &pw, const QString &lw)
{
    if(type==MdCharmGlobal::ShortcutInsertPicture)
        QMessageBox::warning(this, pw, pw);
    else
        QMessageBox::warning(this, lw, lw);
}

void InsertLinkOrPictureDialog::insertLocalFileSlot()
{
    QString filePath = QFileDialog::getOpenFileName(this, tr("Select a image"), proDir, tr("All(*.*)"));
    if(filePath.isEmpty())
        return;
    QFileInfo fileInfo(filePath);
    ui->textLineEdit->setText(fileInfo.fileName());
    filePath = fileInfo.absoluteFilePath();
    if(!proDir.isEmpty()){
        QDir current(proDir);
        filePath = current.relativeFilePath(filePath);
    }
    ui->urlLineEdit->setText(filePath);
}

const QString InsertLinkOrPictureDialog::getText() const
{
    return ui->textLineEdit->text();
}

const QString InsertLinkOrPictureDialog::getUrl() const
{
    return ui->urlLineEdit->text();
}

const QString InsertLinkOrPictureDialog::getTitle() const
{
    return ui->titleLineEdit->text();
}

const QString InsertLinkOrPictureDialog::getWidth() const
{
    return ui->widthLineEdit->text();
}

const QString InsertLinkOrPictureDialog::getHeight() const
{
    return ui->heightLineEdit->text();
}
