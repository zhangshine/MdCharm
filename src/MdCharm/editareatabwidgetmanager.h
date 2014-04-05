#ifndef EDITAREATABWIDGETMANAGER_H
#define EDITAREATABWIDGETMANAGER_H

#include <QWidget>
#include <QLabel>
#include <QSplitter>
#include <QFileSystemWatcher>
#include <QUrl>
#include <QTabWidget>
#include <QHBoxLayout>

#include "editareawidget.h"

class EditAreaTabWidget;
class EditAreaWidget;
class MarkdownEditAreaWidget;
class MdCharmForm;
class Configuration;

class EditAreaTabWidgetManager : public QWidget
{
    Q_OBJECT
public:
    explicit EditAreaTabWidgetManager(MdCharmForm *mainForm);
    MarkdownEditAreaWidget* addMarkdownEditAreaWidget(const QString &filePath,
                                                      const QUrl &baseUrl,
                                                      int id);
    EditAreaTabWidget* getCurrentTabWidget();
    EditAreaWidget* getCurrentWidget();
    void setCurrentEditAreaWidget(EditAreaWidget *eaw);
    QList<EditAreaWidget*> findEditAreaWidgetByFilePath(const QString &path);
    EditAreaTabWidget* findEditAreaTabWidget(EditAreaWidget *eaw);
    void updateConfiguration();
    void switchPreview(int type);
    QString getCurrentTabTabText();
    QString getTabTabText(EditAreaWidget *tab);
    void updateSpellCheckOption(bool isCheck);
    bool saveAllBeforeClose();
    void saveTabsState();
    void restoreTabsState(const QList<StateModel> &sml);
    void removeFromFileWatcher(const QString &filePath);
    void addToFileWatcher(const QString &filePath);
protected:
    virtual void resizeEvent(QResizeEvent *e);
signals:
    void currentChanged();
    void addToRecentFileList(QString);
    void updateActions();
    void showStatusMessage(const QString &msg);
public slots:
    void closeCurrentTab();
    void checkFileStatusWhenMainWindowActived();
    bool saveFile();
    bool saveFile(EditAreaWidget *editArea);
    void saveFileAs();
    void changeSyncScrollBarSetting(bool sync);
    void deleteFileAndTab(const QString &filePath);
    void renameFileAndTab(const QString &original, const QString &current);
    EditAreaWidget* addNewTabWidget(const QString &filePath);
    void showFind();
private slots:
    void updateStatus();
    void fileChangedSlot(const QString filePath);
    void addToFileChangedPendingList(const QString &path);
    void fileChangedNotify(const QString &path);
    void checkViewStatus();
    void updateCurrentTabWidget();
    void moveToOtherViewSlot(int index);
    void cloneToOtherViewSlot(int index);
private:
    void removeTabs(QList<EditAreaWidget *> tabs);
    void removeOneTab(EditAreaWidget* target);
    QList<EditAreaWidget*> getAllEditAreaWidgets();
    void updateTabText(EditAreaWidget *editArea, const QString &fileName);
    EditAreaWidget* findInEditAreaTabWidget(EditAreaTabWidget *view, const QString &filePath);
    EditAreaTabWidget* getIndexView(int index);
    EditAreaWidget* addNewTabWidget(EditAreaTabWidget *view, const QString &filePath);
    MarkdownEditAreaWidget* addMarkdownEditAreaWidget(EditAreaTabWidget *view,
                                                      const QString &filePath,
                                                      const QUrl &baseUrl,
                                                      int id);
private:
    QFileSystemWatcher *fileWatcher;
    QSplitter *mainSplitter;
    MdCharmForm *mainForm;
    QLabel *bgLabel;

    QStringList fileChangedPendingList;
    QList<EditAreaTabWidget *> views;
    QHBoxLayout *layout;

    Configuration *conf;

    EditAreaTabWidget *currentTabWidget;

    int newFileId;
};

#endif // EDITAREATABWIDGETMANAGER_H
