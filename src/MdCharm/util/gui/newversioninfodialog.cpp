#include "newversioninfodialog.h"
#include "markdowntohtml.h"

#include <QDesktopServices>

NewVersionInfoDialog::NewVersionInfoDialog(const QString &version, const QString &revision, const QString &infoMarkdown, QWidget *parent) :
    QDialog(parent, Qt::WindowTitleHint|Qt::WindowSystemMenuHint), rev(revision)
{
    setWindowTitle(tr("MdCharm %1").arg(version));
    QGridLayout *layout = new QGridLayout;
    layout->setMargin(0);
    layout->setVerticalSpacing(8);
    layout->setHorizontalSpacing(0);
    layout->setSizeConstraint(QLayout::SetNoConstraint);

    std::string html;
    QByteArray content = infoMarkdown.toUtf8();
    MarkdownToHtml::translateMarkdownToHtml(MarkdownToHtml::PHPMarkdownExtra, content.data(), content.length(), html);
    infoBrower = new QTextBrowser(this);
    infoBrower->setFrameShape(QFrame::NoFrame);
    infoBrower->setHtml(QString::fromUtf8(html.c_str(), html.length()));
    layout->addWidget(infoBrower, 0, 0);

    buttonBox = new QDialogButtonBox(this);
    ignoreThisVersionButton = buttonBox->addButton(tr("Ignore This Version"), QDialogButtonBox::RejectRole);
    remindMeLaterPushButton = buttonBox->addButton(tr("Remind Me Later"), QDialogButtonBox::RejectRole);
    downloadPushButton = buttonBox->addButton(tr("Download Now"), QDialogButtonBox::AcceptRole);
    QVBoxLayout *buttonBoxLayout = new QVBoxLayout;
    buttonBoxLayout->setMargin(6);
    buttonBoxLayout->addWidget(buttonBox);
    layout->addLayout(buttonBoxLayout, 1, 0);
    setLayout(layout);
    setModal(true);

    connect(buttonBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(buttonClicked(QAbstractButton*)));

    conf = Configuration::getInstance();
}

void NewVersionInfoDialog::buttonClicked(QAbstractButton *btn)
{
    if(btn==ignoreThisVersionButton){
        conf->setLastIgnoreRevision(rev);
        reject();
    } else if(btn==remindMeLaterPushButton){
        reject();
    } else if(btn==downloadPushButton){
        QDesktopServices::openUrl(QUrl::fromUserInput("http://www.mdcharm.com/download.html"));
        accept();
    }
}
