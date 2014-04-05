#include "projectdockwidget.h"
#include "ui_projectdockwidget.h"
#include "utils.h"
#include "util/gui/renamefiledialog.h"
#include "util/filesystemmodel.h"
#include "util/filesystemtreeview.h"
#include "configuration.h"

ProjectDockWidget::ProjectDockWidget(QWidget *parent) :
    QDockWidget(parent),
    ui(new Ui::ProjectDockWidget)
{
    ui->setupUi(this);
    conf = Configuration::getInstance();
    initGui();
    initMenuAndAction();
    initSignalsAndSlots();
}

void ProjectDockWidget::initGui()
{
    setAcceptDrops(true);
    fileSystemModel = new FileSystemModel(this);
    fileSystemModel->setFilter(QDir::AllEntries|QDir::NoDotAndDotDot);
    projectTreeView = new FileSystemTreeView(this);
    ui->verticalLayout->addWidget(projectTreeView);
    projectTreeView->setContextMenuPolicy(Qt::CustomContextMenu);
    projectTreeView->setModel(fileSystemModel);

    projectTreeView->setRootIsDecorated(true);
    projectTreeView->setUniformRowHeights(true);
    projectTreeView->setTextElideMode(Qt::ElideNone);
    projectTreeView->setAttribute(Qt::WA_MacShowFocusRect);

    projectTreeView->setHeaderHidden(true);
    projectTreeView->setEditTriggers(QAbstractItemView::NoEditTriggers);

#ifdef QT_V5
    projectTreeView->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
#else
    projectTreeView->header()->setResizeMode(QHeaderView::ResizeToContents);
#endif
    projectTreeView->header()->setStretchLastSection(false);

}

void ProjectDockWidget::initMenuAndAction()
{
    fileMenu = new QMenu(this);
    directoryMenu = new QMenu(this);
    addNewAction = new QAction(tr("Add New..."), this);
    showInExplorerAction = new QAction(tr("Show in Explorer"), this);
    openFileAction = new QAction(tr("Open file"), this);
    renameFileAction = new QAction(tr("Rename..."), this);
    deleteFileAction = new QAction(tr("Delete..."), this);
}

void ProjectDockWidget::initSignalsAndSlots()
{
    connect(projectTreeView, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(showContextMenu(QPoint)));
    connect(openFileAction, SIGNAL(triggered()),
            this, SLOT(openFile()));
    connect(showInExplorerAction, SIGNAL(triggered()),
            this, SLOT(showInExplorer()));
    connect(addNewAction, SIGNAL(triggered()),
            this, SLOT(createNewAux()));
    connect(renameFileAction, SIGNAL(triggered()),
            this, SLOT(renameFile()));
    connect(deleteFileAction, SIGNAL(triggered()),
            this, SLOT(deleteFile()));
    connect(projectTreeView, SIGNAL(doubleClicked(QModelIndex)),
            this, SLOT(doubleClickedSlot(QModelIndex)));
    connect(this, SIGNAL(visibilityChanged(bool)),
            this, SLOT(visibleChange(bool)));
    connect(fileSystemModel->getFileWatcher(), SIGNAL(directoryChanged(QString)),
            this, SLOT(directoryChanged(QString)));
}

void ProjectDockWidget::showContextMenu(const QPoint &point)
{
    QModelIndex modelIndex = projectTreeView->indexAt(point);
    if(!modelIndex.isValid())
        return;
    QFileInfo fileInfo = fileSystemModel->fileInfo(modelIndex);
    if(fileInfo.isDir())
    {
        prepareDirectoryMenu(fileInfo.absoluteFilePath());
        directoryMenu->exec(projectTreeView->mapToGlobal(point));
    } else if(fileInfo.isFile()&&Utils::isMarkdownFile(fileInfo.absoluteFilePath())){//markdown
        prepareFileMenu(fileInfo.absoluteFilePath());
        fileMenu->exec(projectTreeView->mapToGlobal(point));
    }
}

void ProjectDockWidget::setProjectDir(const QString &dirString)
{
    if(dirString.isEmpty() || dirString.trimmed().isEmpty())
        return;
    projectDir = dirString;

    fileSystemModel->setRootPath(dirString);
}

QString ProjectDockWidget::getProjectDir()
{
    return projectDir;
}

void ProjectDockWidget::closeEvent(QCloseEvent *event)
{
    conf->setProjectDockWidgetVisible(false);
    QDockWidget::closeEvent(event);
}

void ProjectDockWidget::keyPressEvent(QKeyEvent *event)
{
    if(event->key()==Qt::Key_Delete){
        QModelIndex mi = projectTreeView->currentIndex();
        if(!mi.isValid())
            return;
        QFileInfo fileInfo = fileSystemModel->fileInfo(mi);
        if(fileInfo.isFile()){
            event->accept();
            QString filePath = fileInfo.absoluteFilePath();
            emit deleteFileSignal(filePath);
        }
    }
}

ProjectDockWidget::~ProjectDockWidget()
{
    delete ui;
}

void ProjectDockWidget::prepareDirectoryMenu(const QString &dirPath)
{
    directoryMenu->clear();
    addNewAction->setData(dirPath);;
    directoryMenu->addAction(addNewAction);
    showInExplorerAction->setData(dirPath);
    directoryMenu->addAction(showInExplorerAction);
}

void ProjectDockWidget::prepareFileMenu(const QString &filePath)
{
    fileMenu->clear();
    openFileAction->setData(filePath);
    fileMenu->addAction(openFileAction);
    showInExplorerAction->setData(filePath);
    fileMenu->addAction(showInExplorerAction);
    renameFileAction->setData(filePath);;
    fileMenu->addAction(renameFileAction);
    deleteFileAction->setData(filePath);
    fileMenu->addAction(deleteFileAction);
}

void ProjectDockWidget::openFile()
{
    emit openTheFile(openFileAction->data().toString());
}

void ProjectDockWidget::createNewAux()
{
    emit createNewFile(addNewAction->data().toString());
}

void ProjectDockWidget::showInExplorer()
{
    Utils::showInGraphicalShell(showInExplorerAction->data().toString());
}

void ProjectDockWidget::doubleClickedSlot(const QModelIndex &index)
{
    QFileInfo fileInfo = fileSystemModel->fileInfo(index);
    if(fileInfo.isFile()&&Utils::isMarkdownFile(fileInfo.absoluteFilePath()))
    {
        emit openTheFile(fileInfo.absoluteFilePath());
    }
}

void ProjectDockWidget::visibleChange(bool visible)
{
    if(visible)
        conf->setProjectDockWidgetVisible(true);
}

void ProjectDockWidget::directoryChanged(QString dir)
{
    FileSystemTreeViewState state;
    projectTreeView->saveState(state);
    fileSystemModel->directoryChanged(dir);
    projectTreeView->loadState(state);
}

void ProjectDockWidget::deleteFile()
{
    emit deleteFileSignal(deleteFileAction->data().toString());
}

void ProjectDockWidget::renameFile()
{
    RenameFileDialog rfd(renameFileAction->data().toString());
    if(QDialog::Accepted==rfd.exec()){
        emit renameFileSignal(renameFileAction->data().toString(), rfd.getNewFilePath());
    }
}

//------------------- Drag and Drop ----------------------------------------
void ProjectDockWidget::dragEnterEvent(QDragEnterEvent *e)
{
    if(e->mimeData()->hasUrls())
        e->acceptProposedAction();
}

void ProjectDockWidget::dropEvent(QDropEvent *e)
{
    if(e->mimeData()->hasUrls()){
        QUrl url = e->mimeData()->urls().first();
        if(!url.isLocalFile())
            return;
        QFileInfo fileInfo(url.toLocalFile());
        if(fileInfo.isDir())
            setProjectDir(fileInfo.absoluteFilePath());
    }
}
