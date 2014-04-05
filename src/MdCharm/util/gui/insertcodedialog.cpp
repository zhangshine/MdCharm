#include "insertcodedialog.h"
#include "ui_insertcodedialog.h"

InsertCodeDialog::InsertCodeDialog(QWidget *parent) :
    QDialog(parent, Qt::WindowTitleHint|Qt::WindowSystemMenuHint),
    ui(new Ui::InsertCodeDialog)
{
    ui->setupUi(this);
    QStringList supportLan;
    supportLan << "bash" << "c" << "cpp" << "cs"
               << "css" << "diff" << "http" << "html" << "ini"
               << "java" << "javascript" << "json"
               << "markdown" << "perl" << "php"
               << "python" << "ruby" << "sql" << "xml";
    ui->codeTypeComboBox->addItems(supportLan);
    connect(ui->buttonBox, SIGNAL(rejected()),
            this, SLOT(close()));
    connect(ui->buttonBox, SIGNAL(accepted()),
            this, SLOT(accept()));
}

QString InsertCodeDialog::getCodeType()
{
    return ui->codeTypeComboBox->currentText();
}

InsertCodeDialog::~InsertCodeDialog()
{
    delete ui;
}
