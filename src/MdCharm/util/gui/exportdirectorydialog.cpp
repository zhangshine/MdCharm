#include "exportdirectorydialog.h"
#include "ui_exportdirectorydialog.h"

#include <QStandardItemModel>
#include <QPrinter>
#include <QDebug>
#include <QTimer>
#include <QMessageBox>

#include "utils.h"
#include "configuration.h"
#include "markdowntohtml.h"

ExportDirectoryDialog::ExportDirectoryDialog(QWidget *parent, const QString &dirPath) :
    QDialog(parent, Qt::WindowTitleHint|Qt::WindowSystemMenuHint),
    ui(new Ui::ExportDirectoryDialog)
{
    ui->setupUi(this);
    webView = new QWebView;
    timer = new QTimer(this);
    timer->setSingleShot(true);
    if(!dirPath.isEmpty()){
        ui->dirPathLineEdit->setText(dirPath);
        ui->browerPushButton->setEnabled(false);
    }
    ui->dirPathLineEdit->setReadOnly(true);
    ui->seperateHtmlRadioButton->setChecked(true);
    ui->seperateCssAndHtmlcheckBox->setEnabled(false);
    ui->keepDirCheckBox->setChecked(true);
    ui->exportPathWidget->setHidden(true);
    m_model = new QStandardItemModel(this);
    clearModel();
    ui->filesTreeView->setModel(m_model);
    ui->filesTreeView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_conf = Configuration::getInstance();

    connect(ui->browerPushButton, SIGNAL(clicked()), this, SLOT(browerSourceDirSlot()));
    connect(ui->upPushButton, SIGNAL(clicked()), this, SLOT(moveUp()));
    connect(ui->downPushButton, SIGNAL(clicked()), this, SLOT(moveDown()));
    connect(ui->removePushButton, SIGNAL(clicked()), this, SLOT(removeOne()));
    connect(ui->suddirCheckBox, SIGNAL(clicked()), this, SLOT(fillData()));
    connect(ui->seperatePDFRadioButton, SIGNAL(toggled(bool)), ui->seperateCssAndHtmlcheckBox, SLOT(setDisabled(bool)));
    connect(ui->singlePDFRadioButton, SIGNAL(toggled(bool)), ui->seperateCssAndHtmlcheckBox, SLOT(setDisabled(bool)));
    connect(ui->keepDirCheckBox, SIGNAL(toggled(bool)), ui->exportPathWidget, SLOT(setHidden(bool)));
    connect(ui->exportPushButton, SIGNAL(clicked()), this, SLOT(exportBtnSlot()));
    connect(ui->exportPathBrowerPushButton, SIGNAL(clicked()), this, SLOT(exportPathBrowerSlot()));
    connect(this, SIGNAL(exportFinish()), this, SLOT(exportFinishSlot()));
    connect(this, SIGNAL(exportNext()), this, SLOT(exportOneByOne()));
}

ExportDirectoryDialog::~ExportDirectoryDialog()
{
    delete ui;
    webView->deleteLater();
}

void ExportDirectoryDialog::browerSourceDirSlot()
{
    QString dirStr = Utils::getExistingDirectory(this, tr("Select the directory"));
    if(dirStr.isEmpty())
        return;
    ui->dirPathLineEdit->setText(dirStr);
    fillData();
}

void ExportDirectoryDialog::exportPathBrowerSlot()
{
    QString dirStr = Utils::getExistingDirectory(this, tr("Select the directory"));
    if(dirStr.isEmpty())
        return;
    ui->exportLineEdit->setText(dirStr);
}

void ExportDirectoryDialog::fillData()
{
    QString dirStr = ui->dirPathLineEdit->text();
    if(dirStr.isEmpty() || dirStr.trimmed().isEmpty())
        return;
    clearModel();
    QFileInfoList fileInfoList = Utils::listAllFileInDir(dirStr,
                                                         m_conf->getFileFilter(Configuration::MarkdownFile),
                                                         QDir::NoFilter,
                                                         ui->suddirCheckBox->isChecked()
                                                         );
    foreach (QFileInfo fi, fileInfoList) {
        addFile(fi.filePath());
    }
}

void ExportDirectoryDialog::moveDown()
{
    QModelIndex index = ui->filesTreeView->currentIndex();
    int row = index.row();
    if(!index.isValid() || row>=m_model->rowCount()-1)
        return;
    m_model->insertRow(row+1, m_model->takeRow(row));
    ui->filesTreeView->setCurrentIndex(m_model->index(row+1, 0));
}

void ExportDirectoryDialog::removeOne()
{
    QModelIndex index = ui->filesTreeView->currentIndex();
    if(!index.isValid())
        return;
    m_model->removeRow(index.row());
}

void ExportDirectoryDialog::exportBtnSlot()
{
    if(!ui->keepDirCheckBox->isChecked() && ui->exportLineEdit->text().isEmpty()){
        QMessageBox::warning(this, tr("Select export path"), tr("Please select an export path or check \"Keep Directory Struct\" checkbox"));
        return;
    }
    ui->exportPushButton->setEnabled(false);
    timer->singleShot(50, this, SLOT(startExport()));
}

void ExportDirectoryDialog::startExport()
{
    if(htmlTemplate.isEmpty() || htmlTemplate.trimmed().isEmpty()){
        htmlTemplate = Utils::getHtmlTemplate();
        cssTemplate = m_conf->getMarkdownCSS();
    }
    pendingFile = getFiles();
    if(ui->seperateHtmlRadioButton->isChecked()){
        while(!pendingFile.isEmpty()){
            QFileInfo fi(pendingFile.takeFirst());
            QString fileSavePath = ui->keepDirCheckBox->isChecked()
                    ? fi.path()+"/"+fi.baseName()+".html"
                    : ui->exportLineEdit->text()+"/"+fi.baseName()+".html";
            QString fileContent = htmlTemplate.arg(cssTemplate,
                                                   "",
                                                   "",
                                                   Utils::translateMarkdown2Html(m_conf->getMarkdownEngineType(), Utils::readFile(fi.absoluteFilePath())));
            Utils::saveFile(fileSavePath, fileContent.toUtf8());
        }

    } else if(ui->singleHtmlRadioButton->isChecked()){
        QStringList fileContentList;
        while(!pendingFile.isEmpty()){
            QFileInfo fi(pendingFile.takeFirst());
            fileContentList.append(Utils::translateMarkdown2Html(m_conf->getMarkdownEngineType(), Utils::readFile(fi.absoluteFilePath())));
        }
        QString sumAll = fileContentList.join("<hr>");
        QString fileSavePath = Utils::getSaveFileName("*.html", this, tr("Select export path"), ui->dirPathLineEdit->text(), tr("Html files(*.html)"));
        if(fileSavePath.isEmpty())
            return;
        QString fileContent = htmlTemplate.arg(cssTemplate,
                                               "",
                                               "",
                                               sumAll);
        Utils::saveFile(fileSavePath, fileContent.toUtf8());
    } else if(ui->seperatePDFRadioButton->isChecked()){
        exportOneByOne();
    } else if(ui->singlePDFRadioButton->isChecked()){
        QStringList fileContentList;
        while(!pendingFile.isEmpty()){
            QFileInfo fi(pendingFile.takeFirst());
            fileContentList.append(Utils::translateMarkdown2Html(m_conf->getMarkdownEngineType(), Utils::readFile(fi.absoluteFilePath())));
        }
        QString sumAll = fileContentList.join("<hr>");
        QString fileSavePath = Utils::getSaveFileName("*.pdf", this, tr("Select export path"), ui->dirPathLineEdit->text(), tr("PDF files(*.pdf)"));
        if(fileSavePath.isEmpty())
            return;
        pdfOutputFilPath = fileSavePath;
        QString fileContent = htmlTemplate.arg(cssTemplate,
                                               "",
                                               "",
                                               sumAll);
        exportOneByOne(fileContent);
    }
}

void ExportDirectoryDialog::exportOneByOne(const QString &content)
{
    if(content.isEmpty()){
        if(pendingFile.isEmpty()){
            emit exportFinish();
            return;
        }
        QFileInfo fi(pendingFile.takeFirst());
        QString fileSavePath = ui->keepDirCheckBox->isChecked()
                ? fi.path() + "/" + fi.baseName() + ".pdf"
                : ui->exportLineEdit->text()+"/"+fi.baseName()+".pdf";
        QString fileContent = htmlTemplate.arg(cssTemplate,
                                               "",
                                               "",
                                               Utils::translateMarkdown2Html(m_conf->getMarkdownEngineType(), Utils::readFile(fi.absoluteFilePath())));
        pdfOutputFilPath = fileSavePath;
        exportOne(fileContent);
    } else {
        exportOne(content);
    }
}

void ExportDirectoryDialog::exportOne(const QString &content)
{
    if(pdfOutputFilPath.isEmpty())
        return;
    connect(webView, SIGNAL(loadFinished(bool)), this, SLOT(loadFinish()), Qt::UniqueConnection);
    webView->setHtml(content, QUrl::fromUserInput(ui->dirPathLineEdit->text()));
}

void ExportDirectoryDialog::loadFinish()
{
    disconnect(webView, SIGNAL(loadFinished(bool)), this, SLOT(loadFinish()));
    QPrinter printer;
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(pdfOutputFilPath);
    printer.setCreator("MdCharm (http://www.mdcharm.com/)");
    webView->print(&printer);
    pdfOutputFilPath.clear();
    emit exportNext();
}

void ExportDirectoryDialog::exportFinishSlot()
{
    ui->exportPushButton->setEnabled(true);
}

void ExportDirectoryDialog::moveUp()
{
    QModelIndex index = ui->filesTreeView->currentIndex();
    int row = index.row();
    if(!index.isValid()||row<=0)
        return;
    m_model->insertRow(row-1, m_model->takeRow(row));
    ui->filesTreeView->setCurrentIndex(m_model->index(row-1, 0));
}

void ExportDirectoryDialog::clearModel()
{
    m_model->clear();
    m_model->setHorizontalHeaderLabels(QStringList()<<tr("File Path"));
}

QStringList ExportDirectoryDialog::getFiles() const
{
    QStringList files;
    for(int i=0; i<m_model->rowCount(); i++){
        QModelIndex index = m_model->index(i, 0);
        if(index.isValid()){
            files << index.data().toString();
        }
    }
    return files;
}

void ExportDirectoryDialog::addFile(const QString &filePath)
{
    m_model->appendRow(new QStandardItem(filePath));
}
