#include "aboutmdcharmdialog.h"
#include "ui_aboutdialog.h"
#include "version.h"

AboutMdCharmDialog::AboutMdCharmDialog(QWidget *parent) :
    QDialog(parent, Qt::WindowTitleHint|Qt::WindowSystemMenuHint),
    ui(new Ui::AboutMdCharmDialog)
{
    ui->setupUi(this);
//    setAttribute(Qt::WA_DeleteOnClose);
    closePushButton = ui->closePushButton;
    versionLabel = ui->versionLabel;

    versionLabel->setOpenExternalLinks(true);
    versionLabel->setText(QString("<h4>"
                                  "Version %2,"
                                  "Built on %1,"
                                  "From revision %3.<br/>"
                                  "<a href=\"http://www.mdcharm.com/\">http://www.mdcharm.com</a><br/>"
                                  "Copyright &copy; 2012-2013. All Rights Reserved."
                                  "</h4>")
                            .arg(QString::fromLatin1(BUILT_TIME_STR))
                            .arg(QString::fromLatin1(VERSION_STR))
                            .arg(QString::fromLatin1(REVISION_STR).left(10)));

    QObject::connect(closePushButton, SIGNAL(clicked()),
                     this, SLOT(close()));
}

AboutMdCharmDialog::~AboutMdCharmDialog()
{
    delete ui;
}
