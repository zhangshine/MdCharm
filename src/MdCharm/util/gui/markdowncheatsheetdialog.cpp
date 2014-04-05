#include <QHBoxLayout>

#include "markdowncheatsheetdialog.h"
#include "configuration.h"
#include "markdowntohtml.h"

MarkdownCheatSheetDialog::MarkdownCheatSheetDialog(QWidget *parent) :
    QDialog(parent, Qt::WindowTitleHint|Qt::WindowSystemMenuHint|Qt::WindowMinMaxButtonsHint|Qt::WindowCloseButtonHint)
{
    webView = new MarkdownWebView(this);
    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->setMargin(0);
    setLayout(mainLayout);
    mainLayout->addWidget(webView);
    resize(800, 600);
    Configuration *conf = Configuration::getInstance();
    QFile htmlTemplate(":/markdown/markdown.html");
    if(!htmlTemplate.open(QIODevice::ReadOnly))
    {
        Utils::showFileError(htmlTemplate.error(), ":/markdown/markdown.html");
        return;
    }
    QString htmlContent = htmlTemplate.readAll();
    htmlTemplate.close();

    QFile cheatSheetMdFile(":/markdown/markdown_cheat_sheet.md");
    cheatSheetMdFile.open(QFile::ReadOnly);
    QByteArray content = cheatSheetMdFile.readAll();
    std::string textResult;
    MarkdownToHtml::translateMarkdownToHtml(MarkdownToHtml::PHPMarkdownExtra, content.data(), content.length(), textResult);

    htmlContent = htmlContent.arg(conf->getMarkdownCSS())
                       .arg("")
                       .arg("")
                       .arg(QString::fromUtf8(textResult.c_str(), textResult.length()));
    webView->setHtml(htmlContent);
    cheatSheetMdFile.close();
}

MarkdownCheatSheetDialog::~MarkdownCheatSheetDialog()
{
}

void MarkdownCheatSheetDialog::showAndPopup()
{
    show();
    raise();
    activateWindow();
    setWindowState((windowState() & ~Qt::WindowMinimized) | Qt::WindowActive);
}
