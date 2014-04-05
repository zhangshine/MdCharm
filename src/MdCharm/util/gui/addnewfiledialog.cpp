#include "addnewfiledialog.h"
#include "ui_addnewfiledialog.h"

AddNewFileDialog::AddNewFileDialog(QWidget *parent) :
    QDialog(parent, Qt::WindowTitleHint|Qt::WindowSystemMenuHint),
    ui(new Ui::AddNewFileDialog)
{
    ui->setupUi(this);
    parentDirLabel = ui->parentDirLabel;
    fileNameLineEdit = ui->fileNameLineEdit;
    buttonBox = ui->buttonBox;

    connect(buttonBox, SIGNAL(accepted()),
            this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()),
            this, SLOT(reject()));
}

void AddNewFileDialog::setParentDir(const QString &dirPath)
{
    QString dir = dirPath;
    dir.append("/");
#ifdef Q_OS_WIN
    dir.replace("/","\\");//keep this
#endif
    parentDirLabel->setText(dir);
}

QString AddNewFileDialog::getFileName()
{
    return fileNameLineEdit->text();
}

AddNewFileDialog::~AddNewFileDialog()
{
    delete ui;
}
