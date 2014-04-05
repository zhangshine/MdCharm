#ifndef EDITAREATABWIDGET_H
#define EDITAREATABWIDGET_H

#include <QTabWidget>
#include <QFileSystemWatcher>
#include <QMenu>
#include <QTabBar>

#include "utils.h"

class MarkdownEditAreaWidget;
class EditAreaWidget;
class MdCharmForm;
class EditAreaTabWidget;
class EditAreaTabWidgetManager;

class EditAreaTabBar : public QTabBar
{
    Q_OBJECT
public:
    explicit EditAreaTabBar(EditAreaTabWidget *parent=0);
signals:
    void closeTab(int index);
    void closeOtherTabs(int remainIndex);
    void closeAllTabs();
    void cloneToOtherView(int index);
    void moveToOtherView(int index);

protected:
    virtual void mouseReleaseEvent(QMouseEvent *event);
private slots:
    void contextMenuSlot(const QPoint &pos);
    void closeActionSlot();
    void closeAllActionSlot();
    void closeOtherActionSlot();
    void cloneToOtherViewSlot();
    void moveToOtherViewSlot();
private:
    QMenu *menu;
    QAction *closeAction;
    QAction *closeAllAction;
    QAction *closeOthersAction;
    QAction *cloneToOtherViewAction;
    QAction *moveToOtherViewAction;
    EditAreaTabWidget *tabWidget;

    int tabIndex;
};

class EditAreaTabWidget : public QTabWidget
{
    Q_OBJECT
public:
    explicit EditAreaTabWidget(MdCharmForm *mainForm, EditAreaTabWidgetManager *parent = 0);
    bool saveFile(EditAreaWidget *editArea);
    void saveFileAs(EditAreaWidget *editArea);
    MdCharmGlobal::SaveFileOptions saveBeforeClose(EditAreaWidget *editArea, const QString name);
    void closeIfExist(const QString &filePath);
    void renameFile(const QString &original, const QString &current);
    int addMarkdownTab(MarkdownEditAreaWidget* tab, const QIcon &icon, const QString &label);
    int addEditAreaTab(EditAreaWidget *tab, const QIcon &icon, const QString &label);
private:
    void initSignalsAndSlots();
    int addTab(QWidget *widget, const QIcon &icon, const QString &label);
    int addTab(QWidget *widget, const QString &label);
protected:

private:
    QStringList fileChangedPendingList;
    EditAreaTabBar *tabBar;
    MdCharmForm *mainForm;
    EditAreaTabWidgetManager *manager;
signals:
    void focused();
    void cloneToOtherView(int index);
    void moveToOtherView(int index);
private slots:
    void closeTabWidgets(const QList<EditAreaWidget *> &widgets);
    
public slots:
    QList<EditAreaWidget *> listTabWidgets();
    bool closeOneTab(int index);
    void closeAllTabs();
    void closeOtherTabs(int remain);
    void closeCurrentTab();
};

#endif // EDITAREATABWIDGET_H
