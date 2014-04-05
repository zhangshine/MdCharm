#include <QtGui>
#include <QtCore>
#include <QtWebKit>

#ifdef QT_V5
#include <QtWebKitWidgets>
#endif

#include <cassert>

#include "browereditareawidget.h"
#include "version.h"
#include "configuration.h"
#include "basewebview/basewebview.h"

BrowerWebkitHandler::BrowerWebkitHandler()
{
    conf = Configuration::getInstance();
}

void BrowerWebkitHandler::domReady()
{
    updateRecentFiles();
}

void BrowerWebkitHandler::updateRecentFiles()
{
    QStringList rf = conf->getRecentFileList();
    QString strList = rf.join(QString::fromLatin1("|"));
#ifdef Q_OS_WIN
    strList.replace('/','\\');//show file list in html, keep this
#endif
    emit updateRecentFileList(strList);
}

BrowerWebkitHandler::~BrowerWebkitHandler()
{
}

BrowerEditAreaWidget::BrowerEditAreaWidget(const QString &filePath) :
    EditAreaWidget(filePath, 0)

{
    em.setEditorType(EditorModel::BROWER);
    webkitHandler = new BrowerWebkitHandler;
    brower = new BaseWebView(this);
    brower->setAcceptDrops(false);
    brower->page()->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);
    addJavascriptObject();

    initSignalsAndSlots();
    initContent(filePath);

}

BrowerEditAreaWidget::~BrowerEditAreaWidget()
{
    webkitHandler->deleteLater();
}

void BrowerEditAreaWidget::initContent(const QString &filePath)
{
    QString versionInfo("Version %1 revision %2 (%3)");
    versionInfo = versionInfo.arg(QString::fromLatin1(VERSION_STR))
                             .arg(QString::fromLatin1(REVISION_STR).left(10))
                             .arg(QString::fromLatin1(BUILT_TIME_STR));
    QFile startHereFile(filePath);
    if(!startHereFile.open(QIODevice::ReadOnly|QIODevice::Text))
    {
        return;
    }
    brower->setHtml(QString::fromUtf8(startHereFile.readAll()).arg(versionInfo));
    startHereFile.close();
}

void BrowerEditAreaWidget::initSignalsAndSlots()
{
    connect(brower, SIGNAL(linkClicked(QUrl)),
            this, SLOT(openLinkOutside(QUrl)));
    connect(brower->page()->mainFrame(), SIGNAL(javaScriptWindowObjectCleared()),
            this, SLOT(addJavascriptObject()));
}

EditAreaWidget* BrowerEditAreaWidget::clone()
{
    Q_ASSERT(isEditActionOptionEnabled(AllowSplit));
    return NULL;
}

void BrowerEditAreaWidget::setHtml(const QString &html, const QUrl &baseUrl)
{
    brower->setHtml(html, baseUrl);
}

void BrowerEditAreaWidget::setUrl(const QUrl &url)
{
    brower->setUrl(url);
}

void BrowerEditAreaWidget::resizeEvent(QResizeEvent *event)
{
    brower->resize(event->size());
}

void BrowerEditAreaWidget::setText(const QString &html)
{
    setHtml(html);
}

void BrowerEditAreaWidget::cut(){}

void BrowerEditAreaWidget::copy(){}

void BrowerEditAreaWidget::paste(){}

void BrowerEditAreaWidget::redo(){}

void BrowerEditAreaWidget::undo(){}

void BrowerEditAreaWidget::selectAll(){}

bool BrowerEditAreaWidget::isModified(){ return false; }
void BrowerEditAreaWidget::setModified(bool isModi){Q_UNUSED(isModi)}
bool BrowerEditAreaWidget::isUndoAvailable(){ return false; }
bool BrowerEditAreaWidget::isRedoAvailable(){ return false; }

void BrowerEditAreaWidget::addJavascriptObject()
{
    brower->page()->mainFrame()
            ->addToJavaScriptWindowObject("browerWebkitHandler", webkitHandler);
}

void BrowerEditAreaWidget::openLinkOutside(const QUrl &url)
{
    assert(true);
    QDesktopServices::openUrl(url);
}
