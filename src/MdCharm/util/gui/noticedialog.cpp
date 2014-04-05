#include "noticedialog.h"
#include "ui_noticedialog.h"

NoticeDialog::NoticeDialog(QWidget *parent) :
    QDialog(parent, Qt::WindowTitleHint|Qt::WindowSystemMenuHint),
    ui(new Ui::NoticeDialog)
{
    ui->setupUi(this);
    connect(ui->buttonBox, SIGNAL(accepted()),
            this, SLOT(accept()));
}

void NoticeDialog::setNoticeContent(const QString &content)
{
    ui->noticeLabel->setText(content);
}

NoticeDialog::~NoticeDialog()
{
    delete ui;
}
