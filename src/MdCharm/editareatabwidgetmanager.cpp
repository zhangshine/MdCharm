#include "editareatabwidgetmanager.h"
#include "editareatabwidget.h"
#include "editareawidget.h"
#include "markdowneditareawidget.h"
#include "dock/projectdockwidget.h"
#include "mdcharmform.h"
#include "resource.h"
#include "configuration.h"

#include <QMessageBox>

EditAreaTabWidgetManager::EditAreaTabWidgetManager(MdCharmForm *mainForm) :
    QWidget(mainForm), mainForm(mainForm)
{
    conf = Configuration::getInstance();
    setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding, QSizePolicy::TabWidget));
    layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    setLayout(layout);

    bgLabel = new QLabel(this);
    bgLabel->setText("No files are open<hr><ul><li>Open File with Ctrl+O</li><li>Create New File with Ctrl+N</li></ul>");
    bgLabel->setFont(QFont(conf->getFontFamily(), conf->getFontSize()));
    QSize size = bgLabel->sizeHint();
    bgLabel->setGeometry(geometry().width()/2,geometry().height()/2,size.width(), size.height());

    mainSplitter = new QSplitter(Qt::Horizontal, this);
    layout->addWidget(mainSplitter);

    views.append(new EditAreaTabWidget(mainForm, this));
    views.append(new EditAreaTabWidget(mainForm, this));
    foreach (EditAreaTabWidget *view, views) {
        view->setVisible(false);
        connect(view, SIGNAL(currentChanged(int)), this, SIGNAL(currentChanged()));
        connect(view, SIGNAL(currentChanged(int)), this, SLOT(checkViewStatus()));
        connect(view, SIGNAL(currentChanged(int)), this, SLOT(updateCurrentTabWidget()));
        connect(view, SIGNAL(moveToOtherView(int)), this, SLOT(moveToOtherViewSlot(int)));
        connect(view, SIGNAL(cloneToOtherView(int)), this, SLOT(cloneToOtherViewSlot(int)));
        connect(view, SIGNAL(focused()), this, SLOT(updateCurrentTabWidget()));
        mainSplitter->addWidget(view);
    }

    fileWatcher = new QFileSystemWatcher(this);
    connect(fileWatcher, SIGNAL(fileChanged(QString)), this, SLOT(fileChangedSlot(QString)));

    newFileId = 0;
    currentTabWidget = NULL;
}

MarkdownEditAreaWidget* EditAreaTabWidgetManager::addMarkdownEditAreaWidget(
        const QString &filePath, const QUrl &baseUrl, int id)
{
    return addMarkdownEditAreaWidget(getCurrentTabWidget(), filePath, baseUrl, id);
}
MarkdownEditAreaWidget* EditAreaTabWidgetManager::addMarkdownEditAreaWidget(
        EditAreaTabWidget *view, const QString &filePath, const QUrl &baseUrl, int id)
{
    //we assume file is exist or file path is empty(new file)
    MarkdownEditAreaWidget *newMEAW = new MarkdownEditAreaWidget(mainForm, filePath, baseUrl);
    QString title=tr("Untitled %1").arg(id);
    if(!filePath.isEmpty())
    {
        FileModel fm = newMEAW->getFileModel();
        title = fm.getFileName();
        fileWatcher->addPath(fm.getFileFullPath());
    }
    int index = view->addMarkdownTab(newMEAW, QIcon(Resource::UnmodifiedIconStr), title);
    view->setVisible(true);
    view->setCurrentIndex(index);
    newMEAW->setFocusEditor();
    connect(newMEAW, SIGNAL(updateActions()), this, SLOT(updateStatus()));
    connect(newMEAW, SIGNAL(addToRecentFileList(QString)),
            this, SIGNAL(addToRecentFileList(QString)));
    return newMEAW;
}

void EditAreaTabWidgetManager::checkFileStatusWhenMainWindowActived()
{
    while (!fileChangedPendingList.isEmpty()) {
        fileChangedNotify(fileChangedPendingList.takeFirst());
    }
}

bool EditAreaTabWidgetManager::saveFile()
{
    return saveFile(getCurrentWidget());
}

bool EditAreaTabWidgetManager::saveFile(EditAreaWidget *editArea)
{
    Q_ASSERT(editArea);
    if(!editArea || !editArea->getEditorModel().isEditable())
         return false;
    foreach (EditAreaTabWidget *view, views) {
        if(view->indexOf(editArea)!=-1){
            view->saveFile(editArea);
            //update tab text
            FileModel fm = editArea->getFileModel();
            QList<EditAreaWidget *> find = findEditAreaWidgetByFilePath(fm.getFileFullPath());
            foreach (EditAreaWidget *eaw, find) {
                updateTabText(eaw, fm.getFileName());
            }
        }
    }
    return false;
}

void EditAreaTabWidgetManager::saveFileAs()
{
    EditAreaWidget *editArea = getCurrentWidget();
    Q_ASSERT(editArea);
    if(!editArea || !editArea->getEditorModel().isEditable())
        return;
    foreach (EditAreaTabWidget *view, views) {
        if(view->indexOf(editArea)!=-1){
            view->saveFileAs(editArea);
            //update tab text
            FileModel fm = editArea->getFileModel();
            QList<EditAreaWidget *> find = findEditAreaWidgetByFilePath(fm.getFileFullPath());
            foreach (EditAreaWidget *eaw, find) {
                updateTabText(eaw, fm.getFileName());
            }
        }
    }
    return;
}

void EditAreaTabWidgetManager::changeSyncScrollBarSetting(bool sync)
{
    conf->changeSyncScrollbarSetting(sync);
    QList<EditAreaWidget *> tabs = getAllEditAreaWidgets();
    foreach (EditAreaWidget *eaw, tabs) {
        eaw->changeSyncScrollbarSetting(sync);
    }
}

void EditAreaTabWidgetManager::deleteFileAndTab(const QString &filePath)
{
    foreach (EditAreaTabWidget *view, views) {
        view->closeIfExist(filePath);
    }
    if(!QFile::remove(filePath)){
        QMessageBox::warning(this, tr("Failed to delete this file"), tr("Failed to delete this file"));
    }
}

void EditAreaTabWidgetManager::renameFileAndTab(const QString &original, const QString &current)
{
    fileWatcher->removePath(original);
    if(!QFile::rename(original, current)){//try to rename file
        QMessageBox::warning(this, tr("Failed to rename this file"), tr("Failed to rename this file"));
        fileWatcher->addPath(original);
        return;
    }
    foreach (EditAreaTabWidget *view, views) {
        view->renameFile(original, current);
    }
    fileWatcher->addPath(current);
}

EditAreaWidget* EditAreaTabWidgetManager::addNewTabWidget(const QString &filePath)
{
    return addNewTabWidget(getCurrentTabWidget(), filePath);
}

void EditAreaTabWidgetManager::showFind()
{
    EditAreaWidget *editArea = getCurrentWidget();
    Q_ASSERT(editArea);
    if(!editArea)
        return;
    EditorModel em = editArea->getEditorModel();
    if(em.getEditorType()==EditorModel::MARKDOWN){
        MarkdownEditAreaWidget *mea = qobject_cast<MarkdownEditAreaWidget *>(editArea);
        Q_ASSERT(mea);
        if(!mea)
            return;
        mea->showFind();
        mea->setFocusFinder();
    }
}

EditAreaWidget* EditAreaTabWidgetManager::addNewTabWidget(EditAreaTabWidget *view, const QString &filePath)
{
    //find if file is already open by mdcharm
    QList<EditAreaWidget *> find = findEditAreaWidgetByFilePath(filePath);
    if(!find.isEmpty()){
        setCurrentEditAreaWidget(find.first());
        return find.first();
    }
    //file not opened yet, open it
    QString formatFilePath = filePath;
    formatFilePath.replace("\\", "/");
    QString projectDir = mainForm->getProjectDockWidget()->getProjectDir();
    QUrl baseUrl;
    if(!filePath.isEmpty()) {
        //check file existence
        if(!QFile::exists(filePath)){ // file not exist
            QMessageBox::StandardButton sb = QMessageBox::question(this,
                                                                   tr("Create new file"),
                                                                   tr("%1 doesn't exist. Create it?").arg(filePath),
                                                                   QMessageBox::Yes|QMessageBox::No,
                                                                   QMessageBox::Yes);
            if(sb == QMessageBox::No)
                return NULL;
            //else, try create the file
            QFile newFile(filePath);
            if( !newFile.open(QIODevice::WriteOnly|QIODevice::Text) ){
                QMessageBox::information(this, tr("Create new file"),
                                         tr("Cannot create the file \"%1\"").arg(filePath));
                return NULL;
            }
            newFile.close();
        }
        if(!projectDir.isEmpty() && formatFilePath.startsWith(projectDir))
            baseUrl = QUrl::fromLocalFile(projectDir+"/");
        else
            baseUrl = QUrl::fromLocalFile(filePath);
    }
    MarkdownEditAreaWidget *ret = addMarkdownEditAreaWidget(view, formatFilePath, baseUrl, formatFilePath.isEmpty() ? ++newFileId:0);
    emit updateActions();
    return ret;
}

void EditAreaTabWidgetManager::updateStatus()
{
    EditAreaWidget *editArea = qobject_cast<EditAreaWidget *>(sender());
    Q_ASSERT(editArea);
    if(editArea){
        EditAreaTabWidget *view = findEditAreaTabWidget(editArea);
        if(view){
            view->setTabIcon(view->indexOf(editArea), editArea->isModified() ? QIcon(Resource::ModifiedIconStr) : QIcon(Resource::UnmodifiedIconStr));
        }
    }
    emit updateActions();
}

void EditAreaTabWidgetManager::fileChangedSlot(const QString filePath)
{
    if(!mainForm->isActiveWindow()){
        addToFileChangedPendingList(filePath);
        return;
    }
    //if the main form is active
    fileChangedNotify(filePath);
}

void EditAreaTabWidgetManager::addToFileChangedPendingList(const QString &path)
{
    if(path.isEmpty())
        return;
    if(fileChangedPendingList.isEmpty()||fileChangedPendingList.indexOf(path)==-1)
        fileChangedPendingList.append(path);
}

void EditAreaTabWidgetManager::fileChangedNotify(const QString &path)
{
    QList<EditAreaWidget *> result = findEditAreaWidgetByFilePath(path);
    if(result.isEmpty()){//not found
        fileWatcher->removePath(path);
        return;
    }
    if(QFile::exists(path)){//file content changed
        QMessageBox::StandardButton sb =
                QMessageBox::question(this,
                                      tr("Reload"),
                                      tr("%1\nThis file has been modified by another program.\nDo you want to reload it?").arg(path),
                                      QMessageBox::Yes|QMessageBox::No);
        if(sb==QMessageBox::Yes){
            //reload file
            result.first()->reloadFile();
        }
    } else {//file is renamed or removed
        QMessageBox::StandardButton sb =
                QMessageBox::question(this,
                                      tr("Keep non existing file"),
                                      tr("The file \"%1\" doesn't exist anymore.\n Keep this file in editor?").arg(path),
                                      QMessageBox::Yes|QMessageBox::No);
        if(sb==QMessageBox::No){
            removeTabs(result);
        } else {
            foreach (EditAreaWidget *eaw, result) {
                eaw->setModified(true);
            }
        }
    }
}
QList<EditAreaWidget *> EditAreaTabWidgetManager::findEditAreaWidgetByFilePath(const QString &path)
{
    QList<EditAreaWidget *> result;
    if(path.isEmpty())
        return result;
    foreach (EditAreaTabWidget *view, views) {
        EditAreaWidget *find = findInEditAreaTabWidget(view, path);
        if(find)
            result.append(find);
    }
    return result;
}

EditAreaTabWidget* EditAreaTabWidgetManager::findEditAreaTabWidget(EditAreaWidget *eaw)
{
    if(!eaw)
        return NULL;
    foreach (EditAreaTabWidget *view, views) {
        int index = view->indexOf(eaw);
        if(index!=-1){
            return view;
        }
    }
    Q_ASSERT(0 && "can't be null");
    return NULL;
}

void EditAreaTabWidgetManager::updateConfiguration()
{
    QList<EditAreaWidget *> tabs = getAllEditAreaWidgets();
    foreach (EditAreaWidget *eaw, tabs) {
        eaw->updateConfiguration();
    }
}

void EditAreaTabWidgetManager::switchPreview(int type)
{
    QList<EditAreaWidget *> tabs = getAllEditAreaWidgets();
    foreach (EditAreaWidget *eaw, tabs) {
        eaw->switchPreview(type);
    }
}

QString EditAreaTabWidgetManager::getCurrentTabTabText()
{
    return getTabTabText(getCurrentWidget());
}

QString EditAreaTabWidgetManager::getTabTabText(EditAreaWidget *tab)
{
    Q_ASSERT(tab);
    EditAreaTabWidget *tabWidget = getCurrentTabWidget();
    Q_ASSERT(tabWidget);
    if(!tab || !tabWidget)
        return QString();
    int index = tabWidget->indexOf(tab);
    if(index!=-1){
        return tabWidget->tabText(index);
    }
    return QString();
}

void EditAreaTabWidgetManager::updateSpellCheckOption(bool isCheck)
{
    QList<EditAreaWidget *> tabs = getAllEditAreaWidgets();
    foreach (EditAreaWidget *tab, tabs) {
        if(tab->getEditorModel().getEditorType()==EditorModel::MARKDOWN){
            MarkdownEditAreaWidget *meaw = qobject_cast<MarkdownEditAreaWidget *>(tab);
            if(!meaw)
                continue;
            if(isCheck)
                meaw->enableSpellCheck();
            else
                meaw->disableSpellCheck();
        }
    }
}

bool EditAreaTabWidgetManager::saveAllBeforeClose()
{
    QStringList sl;
    foreach (EditAreaTabWidget *editAreaTabWidget, views) {
        for(int i=0; i<editAreaTabWidget->count(); i++){
            EditAreaWidget *eaw = qobject_cast<EditAreaWidget *>(editAreaTabWidget->widget(i));
            if(!eaw || (!eaw->getFileModel().isUntitled() && sl.indexOf(eaw->getFileModel().getFileFullPath())!=-1))//deal clone tab
                continue;
            if(MdCharmGlobal::Cancel==editAreaTabWidget->saveBeforeClose(eaw, editAreaTabWidget->tabText(i)))
                return false;
            sl.append(eaw->getFileModel().getFileFullPath());
        }
    }
    return true;
}

void EditAreaTabWidgetManager::closeCurrentTab()
{
    EditAreaTabWidget *tabWidget = getCurrentTabWidget();
    if(!tabWidget)
        return;
    tabWidget->closeCurrentTab();
}

void EditAreaTabWidgetManager::removeTabs(QList<EditAreaWidget *> tabs)//delete
{
    foreach (EditAreaWidget *eaw, tabs) {
        removeOneTab(eaw);
    }
    checkViewStatus();//refresh views
}

void EditAreaTabWidgetManager::removeOneTab(EditAreaWidget *target)
{
    Q_ASSERT(target);
    if(!target)
        return;
    EditAreaTabWidget *view = findEditAreaTabWidget(target);
    Q_ASSERT(view);
    if(view){
        view->removeTab(view->indexOf(target));
        target->deleteLater();
    }
}

void EditAreaTabWidgetManager::checkViewStatus()
{
    bool allHide = true;
    foreach (EditAreaTabWidget *view, views) {
        if(view->count()==0&&view->isVisible())
            view->hide();
        if(view->count()>0&&!view->isVisible())
            view->show();
        if(view->count()>0)
            allHide = false;
    }
    bgLabel->setVisible(allHide);
}

void EditAreaTabWidgetManager::updateCurrentTabWidget()
{
    currentTabWidget = qobject_cast<EditAreaTabWidget *>(sender());
    if(currentTabWidget && currentTabWidget->count()<=0)
        currentTabWidget = NULL;
    emit updateActions();
    emit currentChanged();
}

void EditAreaTabWidgetManager::moveToOtherViewSlot(int index)
{
    EditAreaTabWidget *view = qobject_cast<EditAreaTabWidget *>(sender());
    Q_ASSERT(view);
    if(!view)
        return;
    if(index>=view->count())
        return;
    EditAreaWidget *eaw = qobject_cast<EditAreaWidget *>(view->widget(index));
    if(!eaw)
        return;
    EditAreaTabWidget *otherView = NULL;
    foreach (EditAreaTabWidget *v, views) {
        if(v!=view){
            otherView = v;
            break;
        }
    }
    if(!otherView)
        return;
    //find if it exist
    FileModel fm = eaw->getFileModel();
    EditAreaWidget *exist = findInEditAreaTabWidget(otherView, fm.getFileFullPath());
    if(exist){//remove current if exist in other view and return
        view->removeTab(index);
        otherView->setCurrentIndex(otherView->indexOf(exist));
        checkViewStatus();
        return;
    }
    QString tabText = view->tabText(index);
    view->removeTab(index);
    int newIndex = otherView->addEditAreaTab(eaw, eaw->isModified() ? QIcon(Resource::ModifiedIconStr) : QIcon(Resource::UnmodifiedIconStr), tabText);
    otherView->setVisible(true);
    otherView->setCurrentIndex(newIndex);
    currentTabWidget = otherView;
    checkViewStatus();
}

void EditAreaTabWidgetManager::cloneToOtherViewSlot(int index)
{
    EditAreaTabWidget *view = qobject_cast<EditAreaTabWidget *>(sender());
    Q_ASSERT(view);
    if(!view)
        return;
    if(index>=view->count())
        return;
    EditAreaWidget *eaw = qobject_cast<EditAreaWidget *>(view->widget(index));
    if(!eaw)
        return;
    EditAreaTabWidget *otherView = NULL;
    foreach (EditAreaTabWidget *v, views) {
        if(v!=view){
            otherView = v;
            break;
        }
    }
    if(!otherView)
        return;
    //find if it exist
    FileModel fm = eaw->getFileModel();
    EditAreaWidget* exist = findInEditAreaTabWidget(otherView, fm.getFileFullPath());
    if(exist){
        otherView->setCurrentIndex(otherView->indexOf(exist));
        return;
    }
    EditAreaWidget *copy = eaw->clone();
    if(!copy)
        return;
    EditorModel em = copy->getEditorModel();
    if(em.getEditorType()==EditorModel::MARKDOWN){
        MarkdownEditAreaWidget *meaw = qobject_cast<MarkdownEditAreaWidget *>(copy);
        if(meaw){
            connect(meaw, SIGNAL(updateActions()), this, SLOT(updateStatus()));
            connect(meaw, SIGNAL(addToRecentFileList(QString)),
                    this, SIGNAL(addToRecentFileList(QString)));
        }
    }
    int newIndex = otherView->addEditAreaTab(copy, copy->isModified() ? QIcon(Resource::ModifiedIconStr) : QIcon(Resource::UnmodifiedIconStr), view->tabText(index));
    otherView->setVisible(true);
    otherView->setCurrentIndex(newIndex);
    currentTabWidget = otherView;
    checkViewStatus();
}

EditAreaTabWidget* EditAreaTabWidgetManager::getCurrentTabWidget()
{
    if(!currentTabWidget || currentTabWidget->count()<=0)//Notice: if we support delete view make sure it is not null
        return views.first();
    return currentTabWidget;
}

EditAreaWidget* EditAreaTabWidgetManager::getCurrentWidget()
{
    EditAreaTabWidget *currentTabWidget = getCurrentTabWidget();
    if(!currentTabWidget)
        return NULL;
    return qobject_cast<EditAreaWidget *>(currentTabWidget->currentWidget());
}

void EditAreaTabWidgetManager::setCurrentEditAreaWidget(EditAreaWidget *eaw)
{
    Q_ASSERT(eaw);
    if(!eaw)
        return;
    foreach (EditAreaTabWidget *view, views) {
        int index = view->indexOf(eaw);
        if(index!=-1){
            view->setVisible(true);
            view->setFocus();
            view->setCurrentIndex(index);
            MarkdownEditAreaWidget *meaw = qobject_cast<MarkdownEditAreaWidget *>(eaw);
            if(meaw)
                meaw->setFocusEditor();
            break;
        }
    }
}

QList<EditAreaWidget *> EditAreaTabWidgetManager::getAllEditAreaWidgets()
{
    QList<EditAreaWidget *> widgets;
    foreach (EditAreaTabWidget* eatw, views) {
        widgets.append(eatw->listTabWidgets());
    }
    return widgets;
}

void EditAreaTabWidgetManager::updateTabText(EditAreaWidget *editArea, const QString &fileName)
{
    Q_ASSERT(editArea);
    if(!editArea)
        return;
    foreach (EditAreaTabWidget *view, views) {
        int index = view->indexOf(editArea);
        if(index!=-1){
            view->setTabText(index, fileName);
            break;
        }
    }
}

EditAreaWidget* EditAreaTabWidgetManager::findInEditAreaTabWidget(EditAreaTabWidget *view, const QString &filePath)
{
    if(filePath.isEmpty() || view->count()<=0)
        return NULL;
    QString formatStr = QDir::fromNativeSeparators(filePath);
#ifdef Q_OS_WIN
    formatStr = formatStr.toLower();
#endif
    QList<EditAreaWidget *> tabs = view->listTabWidgets();
    foreach (EditAreaWidget *tab, tabs) {
        FileModel fm = tab->getFileModel();
#ifdef Q_OS_WIN
        if(fm.getFileFullPath().toLower()==formatStr){
#else
        if(fm.getFileFullPath()==formatStr){
#endif
            return tab;
        }
    }
    return NULL;
}

void EditAreaTabWidgetManager::saveTabsState()
{
    const QString sessionFilePath = conf->getSessionFilePath();
    QFile sessionFile(sessionFilePath);
    if(!sessionFile.open(QIODevice::ReadWrite|QIODevice::Text|QIODevice::Truncate))
        return;

    EditAreaTabWidget *eatw = getCurrentTabWidget();
    int activeViewIndex = -1;
    foreach(EditAreaTabWidget *view, views){
        if(view->count()>0)
            activeViewIndex++;
        if(view==eatw)
            break;
    }
    if(activeViewIndex==-1)
        activeViewIndex = 0;
    QXmlStreamWriter xmlStreamWriter(&sessionFile);
    //set pretty formating
    xmlStreamWriter.setAutoFormatting(true);
    //write content
    xmlStreamWriter.writeStartDocument();
    xmlStreamWriter.writeStartElement(Utils::AppName);
    xmlStreamWriter.writeStartElement("Session");
    xmlStreamWriter.writeAttribute("activeView", QString::number(activeViewIndex));
    foreach (EditAreaTabWidget *editAreaTabWidget, views) {
        if(editAreaTabWidget->count()<=0)
            continue;
        xmlStreamWriter.writeStartElement("MainView");
        for(int i=0; i<editAreaTabWidget->count(); i++){
            EditAreaWidget *edit = qobject_cast<EditAreaWidget*>(editAreaTabWidget->widget(i));
            if(edit){
                EditorModel em = edit->getEditorModel();
                StateModel sm = edit->getState();
                int isCurrentTab = editAreaTabWidget->currentIndex()==i ? 1 : 0;
                if(em.isEditable()&&sm.isValid()){
                    xmlStreamWriter.writeStartElement("File");
                    xmlStreamWriter.writeAttribute("isCurrentTab", QString::number(isCurrentTab));
                    xmlStreamWriter.writeAttribute("firstVisibleLine", QString::number(sm.getFirstVisibleLine()));
                    xmlStreamWriter.writeAttribute("selectionStart", QString::number(sm.getSelectionStart()));
                    xmlStreamWriter.writeAttribute("selectionEnd", QString::number(sm.getSelectionEnd()));
                    xmlStreamWriter.writeAttribute("verticalScrollBarCurrentValue",
                                                   QString::number(sm.getVerticalScorllBarCurrentValue()));
                    xmlStreamWriter.writeAttribute("verticalScrollBarMaxValue", QString::number(sm.getVerticalScrollBarMaxValue()));
                    xmlStreamWriter.writeAttribute("filename", sm.getFileFullPath());
                    xmlStreamWriter.writeEndElement();//end File
                }
            }
        }

        xmlStreamWriter.writeEndElement();//end MainView
    }
    xmlStreamWriter.writeStartElement("DockWidget");
    xmlStreamWriter.writeStartElement("Project");
    xmlStreamWriter.writeAttribute("dirPath", mainForm->getProjectDockWidget()->getProjectDir());
    xmlStreamWriter.writeEndElement();//end project
    xmlStreamWriter.writeEndElement();//end dock widget

    xmlStreamWriter.writeEndElement();//end Session
    xmlStreamWriter.writeEndElement();//end MdCharm
    xmlStreamWriter.writeEndDocument();//end document

    sessionFile.close();
}

void EditAreaTabWidgetManager::restoreTabsState(const QList<StateModel> &sml)
{
    if(sml.isEmpty())
        return;
    int willIndex = views.first()->count();
    int currentView = 0;
    int count = 0;
    foreach (StateModel sm, sml) {
        if(sm.getViewNum()!=currentView){
            EditAreaTabWidget *sumView = getIndexView(currentView);
            if(sumView && sumView->count()>willIndex)
                sumView->setCurrentIndex(willIndex);
            currentView = sm.getViewNum();
            count = 0;
            willIndex = 0;
        }
        if(!QFile::exists(sm.getFileFullPath()))
            continue;
        QList<EditAreaWidget *> exist = findEditAreaWidgetByFilePath(sm.getFileFullPath());
        EditAreaTabWidget *view = getIndexView(currentView);
        if(exist.isEmpty()){
            EditAreaWidget* eaw = addNewTabWidget(view, sm.getFileFullPath());
            if(eaw){
                eaw->restoreFileState(sm);
            }
        } else {
            EditAreaWidget *src = exist.first();
            EditAreaTabWidget *srcView = findEditAreaTabWidget(src);
            Q_ASSERT(srcView && srcView!=view);
            if(srcView){
                EditAreaWidget *copy = src->clone();
                if(!copy)
                    continue;
                if(copy->getEditorModel().getEditorType()==EditorModel::MARKDOWN){
                    MarkdownEditAreaWidget *meaw = qobject_cast<MarkdownEditAreaWidget *>(copy);
                    if(meaw){
                        connect(meaw, SIGNAL(updateActions()), this, SLOT(updateStatus()));
                        connect(meaw, SIGNAL(addToRecentFileList(QString)),
                                this, SIGNAL(addToRecentFileList(QString)));
                    }
                }
                view->addEditAreaTab(copy,
                                     copy->isModified()?QIcon(Resource::ModifiedIconStr):QIcon(Resource::UnmodifiedIconStr),
                                     srcView->tabText(srcView->indexOf(src)));
                copy->restoreFileState(sm);
            }
        }
        if(sm.isCurrentTab())
            willIndex += count;
        count++;
    }
    EditAreaTabWidget *last = getIndexView(currentView);
    if(last && last->count()>willIndex)
        last->setCurrentIndex(willIndex);
    checkViewStatus();
}

EditAreaTabWidget* EditAreaTabWidgetManager::getIndexView(int index)
{
    if(index>=views.count())
        return views.last();
    return views.value(index);
}

void EditAreaTabWidgetManager::removeFromFileWatcher(const QString &filePath)
{
    fileWatcher->removePath(filePath);
}

void EditAreaTabWidgetManager::addToFileWatcher(const QString &filePath)
{
    fileWatcher->addPath(filePath);
}

void EditAreaTabWidgetManager::resizeEvent(QResizeEvent *e)
{
    QWidget::resizeEvent(e);
    QRect gRect = geometry();
    QSize lSize = bgLabel->sizeHint();
    bgLabel->setGeometry((gRect.width()-lSize.width())/2, (gRect.height()-lSize.height())/2, lSize.width(), lSize.height());
}
