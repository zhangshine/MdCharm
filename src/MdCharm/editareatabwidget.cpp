#include <QtGui>
#include <QtCore>
#include <cassert>

#include "editareatabwidget.h"
#include "editareatabwidgetmanager.h"
#include "markdowneditareawidget.h"
#include "resource.h"
//---------------- EditAreaTabBar ----------------------------------------------
EditAreaTabBar::EditAreaTabBar(EditAreaTabWidget *parent) :
    QTabBar(parent)
{
    tabIndex = -1;
    tabWidget = parent;
    menu = new QMenu(this);
    closeAction = new QAction(tr("Close"), this);
    closeOthersAction = new QAction(tr("Close Others"), this);
    closeAllAction = new QAction(tr("Close All"), this);
    moveToOtherViewAction = new QAction(tr("Move to other view"), this);
    cloneToOtherViewAction = new QAction(tr("Clone to other view"), this);
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(contextMenuSlot(QPoint)));
    connect(closeAction, SIGNAL(triggered()), this, SLOT(closeActionSlot()));
    connect(closeOthersAction, SIGNAL(triggered()),
            this, SLOT(closeOtherActionSlot()));
    connect(closeAllAction, SIGNAL(triggered()),
            this, SLOT(closeAllActionSlot()));
    connect(moveToOtherViewAction, SIGNAL(triggered()),
            this, SLOT(moveToOtherViewSlot()));
    connect(cloneToOtherViewAction, SIGNAL(triggered()),
            this, SLOT(cloneToOtherViewSlot()));
}

void EditAreaTabBar::mouseReleaseEvent(QMouseEvent *event)
{
    if(event->button()==Qt::MiddleButton){
        tabIndex = tabAt(event->pos());
        closeActionSlot();
        event->accept();
    } else {
        QTabBar::mouseReleaseEvent(event);
    }
}

void EditAreaTabBar::contextMenuSlot(const QPoint &pos)
{
    tabIndex = tabAt(pos);
    EditAreaWidget *eaw = qobject_cast<EditAreaWidget *>(tabWidget->widget(tabIndex));
    if(!eaw)
        return;
    menu->clear();
    menu->addAction(closeAction);
    menu->addAction(closeOthersAction);
    menu->addAction(closeAllAction);

    menu->addSeparator();

    if(eaw->isEditActionOptionEnabled(EditAreaWidget::AllowSplit) && !eaw->getFileModel().isUntitled()){
        menu->addAction(cloneToOtherViewAction);
    }
    menu->addAction(moveToOtherViewAction);
    menu->exec(mapToGlobal(pos));
}

void EditAreaTabBar::closeActionSlot()
{
    emit closeTab(tabIndex);
}

void EditAreaTabBar::closeOtherActionSlot()
{
    emit closeOtherTabs(tabIndex);
}

void EditAreaTabBar::closeAllActionSlot()
{
    emit closeAllTabs();
}

void EditAreaTabBar::cloneToOtherViewSlot()
{
    emit cloneToOtherView(tabIndex);
}

void EditAreaTabBar::moveToOtherViewSlot()
{
    emit moveToOtherView(tabIndex);
}

//---------------- EditAreaTabWidget -------------------------------------------
EditAreaTabWidget::EditAreaTabWidget(MdCharmForm *mainForm, EditAreaTabWidgetManager *parent) :
    QTabWidget(parent),
    mainForm(mainForm),
    manager(parent)
{
    //Custom tabbar
    tabBar = new EditAreaTabBar(this);
    setTabBar(tabBar);
    // Style
    setTabsClosable(true);
    setMovable(true);

    initSignalsAndSlots();
}

void EditAreaTabWidget::initSignalsAndSlots()
{
    //--------- TabBar Signals and Slots -----------------------------
    connect(tabBar, SIGNAL(closeTab(int)), this, SLOT(closeOneTab(int)));
    connect(tabBar, SIGNAL(closeOtherTabs(int)), this, SLOT(closeOtherTabs(int)));
    connect(tabBar, SIGNAL(closeAllTabs()), this, SLOT(closeAllTabs()));
    connect(tabBar, SIGNAL(cloneToOtherView(int)), this, SIGNAL(cloneToOtherView(int)));
    connect(tabBar, SIGNAL(moveToOtherView(int)), this, SIGNAL(moveToOtherView(int)));

    connect(this, SIGNAL(tabCloseRequested(int)), this, SLOT(closeOneTab(int)));
}

int EditAreaTabWidget::addMarkdownTab(MarkdownEditAreaWidget *tab, const QIcon &icon, const QString &label)
{
    connect(tab, SIGNAL(showStatusMessage(QString)), manager, SIGNAL(showStatusMessage(QString)));
    if(!tab){
        Q_ASSERT(0 && "this should not be happen");
        return -1;
    }
    connect(tab, SIGNAL(focusInSignal()), this, SIGNAL(focused()));
    return addTab(tab, icon, label);
}

int EditAreaTabWidget::addEditAreaTab(EditAreaWidget *tab, const QIcon &icon, const QString &label)
{
    EditorModel em = tab->getEditorModel();
    if(em.getEditorType()==EditorModel::MARKDOWN){
        return addMarkdownTab(qobject_cast<MarkdownEditAreaWidget *>(tab), icon, label);
    } else {
        Q_ASSERT(0 && "this should not be happen");
        return addTab(tab, icon, label);
    }
}

int EditAreaTabWidget::addTab(QWidget *widget, const QIcon &icon, const QString &label)
{
    return QTabWidget::addTab(widget, icon, label);
}

int EditAreaTabWidget::addTab(QWidget *widget, const QString &label)
{
    return QTabWidget::addTab(widget, label);
}

void EditAreaTabWidget::closeCurrentTab()
{
    closeOneTab(currentIndex());
}

bool EditAreaTabWidget::closeOneTab(int index)
{
    if(index<0||index>=count()) // -1 means no tab
        return false;
    QWidget *tab = widget(index);
    EditAreaWidget *editArea = dynamic_cast<EditAreaWidget *>(tab);
    assert(editArea);
    if(editArea)
    {
        if(MdCharmGlobal::Cancel==saveBeforeClose(editArea, tabText(index)))
            return false;
        FileModel fm = editArea->getFileModel();
        if(!fm.getFileFullPath().isEmpty())
            manager->removeFromFileWatcher(fm.getFileFullPath());
    }
    removeTab(index);
    delete tab;
    return true;
}

void EditAreaTabWidget::closeOtherTabs(int remain)
{
    if(remain>=count())
        return;
    QList<EditAreaWidget *> widgets = listTabWidgets();
    EditAreaWidget* target = qobject_cast<EditAreaWidget *>(widget(remain));
    if(target)
        widgets.removeOne(target);
    closeTabWidgets(widgets);
}

void EditAreaTabWidget::closeAllTabs()
{
    closeTabWidgets(listTabWidgets());
}

QList<EditAreaWidget *> EditAreaTabWidget::listTabWidgets()
{
    QList<EditAreaWidget *> widgets;
    for(int i=0; i<count(); i++){
        EditAreaWidget *eaw = qobject_cast<EditAreaWidget *>(widget(i));
        if(eaw)
            widgets.append(eaw);
    }
    return widgets;
}

void EditAreaTabWidget::closeTabWidgets(const QList<EditAreaWidget *>& widgets)
{
    foreach (QWidget *cur, widgets) {
        int tabIndex = indexOf(cur);
        if(tabIndex<0)
            continue;
        setCurrentIndex(tabIndex);
        if(!closeOneTab(tabIndex))
            break;
    }
}

void EditAreaTabWidget::closeIfExist(const QString &filePath)
{
    QString fp = filePath;
    for(int i=0; i<count(); i++){
        EditAreaWidget *editArea = qobject_cast<EditAreaWidget *>(widget(i));
        FileModel fm = editArea->getFileModel();
#ifdef Q_OS_WIN
        if(fp==fm.getFileFullPath() && fp.toLower()==fm.getFileFullPath().toLower())
#else
        if(fp==fm.getFileFullPath())
#endif
        {
            closeOneTab(i);
            break;
        }
    }
}

void EditAreaTabWidget::renameFile(const QString &original, const QString &current)
{
    int target = -1;
    EditAreaWidget *targetEditArea = NULL;
    for(int i=0; i<count(); i++){
        EditAreaWidget *editArea = qobject_cast<EditAreaWidget *>(widget(i));
        if(!editArea)
            continue;
        FileModel fm = editArea->getFileModel();
        //Since we share FileModel, the file path may be alreay changed in other view so we compare `original` and `current`
#ifdef Q_OS_WIN
        if( (original==fm.getFileFullPath() && original.toLower()==fm.getFileFullPath().toLower())
                || (current==fm.getFileFullPath() && current.toLower()==fm.getFileFullPath().toLower()) )
#else
        if(original==fm.getFileFullPath() || current==fm.getFileFullPath())
#endif
        {
            target = i;
            targetEditArea = editArea;
            break;
        }
    }
    if(targetEditArea && target>=0){
        targetEditArea->changeFilePath(current);
        setTabText(target, targetEditArea->getFileModel().getFileName());
    }
}

MdCharmGlobal::SaveFileOptions EditAreaTabWidget::saveBeforeClose(EditAreaWidget *editArea, const QString name)//True - you can close it, false don't close the tab
{
    assert(editArea);
    EditorModel em = editArea->getEditorModel();
    if(em.getEditorType()>EditorModel::EDITABLE && editArea->isModified())
    {
        QMessageBox::StandardButton sb = QMessageBox::question(this, tr("Save"),
                                 tr("Save File %1 ?").arg(name),
                                 QMessageBox::Save|QMessageBox::Discard|QMessageBox::Cancel);
        if(sb==QMessageBox::Save)
        {
            if(!saveFile(editArea))
                return MdCharmGlobal::Cancel; //not saved, don't close the tab
        }
        else if(sb==QMessageBox::Cancel)
        {
            return MdCharmGlobal::Cancel;
        }
        else
        {
            return MdCharmGlobal::DontSave;
        }
        return MdCharmGlobal::Save;
    }
    return MdCharmGlobal::None;
}
bool EditAreaTabWidget::saveFile(EditAreaWidget *editArea)
{
    if(!editArea || !editArea->getEditorModel().isEditable())
        return false;
    FileModel fm = editArea->getFileModel();
    if(!fm.getFileFullPath().isEmpty())
        manager->removeFromFileWatcher(fm.getFileFullPath());
    bool result = editArea->saveFile();
    fm = editArea->getFileModel();
    if(!fm.getFileFullPath().isEmpty())
        manager->addToFileWatcher(fm.getFileFullPath());
    return result;
}

void EditAreaTabWidget::saveFileAs(EditAreaWidget *editArea)
{
    if(!editArea || !editArea->getEditorModel().isEditable())
        return;
    FileModel fm = editArea->getFileModel();
    if(!fm.getFileFullPath().isEmpty())
        manager->removeFromFileWatcher(fm.getFileFullPath());
    editArea->saveFileAs();
    fm = editArea->getFileModel();
    if(!fm.getFileFullPath().isEmpty())
        manager->addToFileWatcher(fm.getFileFullPath());
}
