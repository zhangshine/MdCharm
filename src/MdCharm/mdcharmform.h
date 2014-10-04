#ifndef MDCHARMFORM_H
#define MDCHARMFORM_H

#include <QMainWindow>
#include <QShortcut>
#include <QTimer>
#include <QActionGroup>
#include <QList>

class EditAreaWidget;
class EditAreaTabWidgetManager;
class CheckUpdates;
class ProjectDockWidget;
class TOCDockWidget;
class StatusBarLabel;
class MarkdownCheatSheetDialog;
class Configuration;
class QLabel;
class MdCharmGlobal;

class MdCharmForm : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MdCharmForm(QWidget *parent = 0);
    ~MdCharmForm();

private:
    void initGui();
    void initMenuContent();
    void initStatusBarContent();
    void initToolBarContent();
    void initDockWidgets();
    void initSignalsAndSlots();
    void initShortcutMatters();
    void setStatusMessage(QString &msg);
    void updateWindowTitle(const QString &title, bool isModified);
    void addStartHerePage();
    void saveMdCharmState();
    void showNotice(const QString &notice);
public:
    void restoreMdCharmState();
    void openArgFiles(QStringList &fileList);
    void checkCodeSyntaxCss();
    ProjectDockWidget* getProjectDockWidget();
protected:
    void dragEnterEvent(QDragEnterEvent *e);
    void dragLeaveEvent(QDragLeaveEvent *e);
    void dropEvent(QDropEvent *e);
    void changeEvent(QEvent *e);
    void closeEvent(QCloseEvent *event);

private:
    //Menu
    QMenuBar *menuBar;
    // File Menu
    QMenu *fileMenu;
    // File Menu Action
    QAction *newAction;
    QAction *openAction;
    QAction *openDirectoryAction;
    QAction *saveAction;
    QAction *saveAsAction;
    QAction *exportAction;

    QMenu *recentFileMenu;

    QAction *printAction;
    QAction *printPreviewAction;

    QAction *quitAction;
    // Edit Menu
    QMenu *editMenu;

    QAction *undoAction;
    QAction *redoAction;

    QAction *cutAction;
    QAction *copyAction;
    QAction *pasteAction;

    QAction *selectAllAction;
    QMenu *findMenu;
    QAction *findAction;
    QAction *findPreviousAction;
    QAction *findNextAction;

    QAction *spellCheckAction;
    // View Menu
    QMenu *viewMenu;
    // Tools Menu
    QMenu *toolsMenu;
    QAction *exportDirAction;
    // Setting Menu
    QMenu *settingsMenu;
    QAction *preferencesAction;
    QActionGroup *previewOptionActionGroup;
    QAction *writeModeAction;
    QAction *writeReadAction;
    QAction *readModeAction;
    QAction *syncScrollbarAction;
    // Help Menu
    QMenu *helpMenu;
    // Help Menu Action
//    QAction *helpAction;
    QAction *markdownCheatSheetAction;
    QAction *checkUpdatesAction;
    QAction *aboutAction;
    // Tool Bar
    QToolBar *toolBar;
    QToolBar *markdownToolBar;
    QToolBar *previewOptionToolBar;
    //Dock Tool Bar
    QToolBar *dockBar;
    //Markdown Toolbar
    QAction *strikeThoughAction;
    // Main Widget
    EditAreaTabWidgetManager *editAreaTabWidgetManager;
    // Status Bar
    StatusBarLabel *lineAndColumnInfoLabel;
    QLabel *overWriteInfoLabel;
    StatusBarLabel *encodingInfoLabel;
    QStatusBar *statusBar;

    //Dock Widgets
    ProjectDockWidget *projectDockWidget;
    TOCDockWidget *tocDockWidget;

    //other
    QClipboard *clipboard;
    Configuration *conf;
    QTimer checkUpdatesTimer;
    CheckUpdates *cu;

    //var area
    bool showCongratulation;

    //Recent File Actions
    enum { MaxRecentFiles = 10 };
    QAction *recentFileActions[MaxRecentFiles];//max=10
    QAction *clearRecentFilesAction;

    //Global Shortcut
    QShortcut *closeTabShortcut;

    MarkdownCheatSheetDialog *mcsd;

    //Window title
    QString appTitle;

    MdCharmGlobal *mdcharmGlobalInstance;
    QList<QAction *> shortcutActions;

public slots:
    EditAreaWidget *addNewTabWidget(const QString &filePath=QString());
    void updateActions();
    void openFiles();
    void openDirectory();
    void exportToHtml(const QString &filePath);
    void exportToPdf(const QString &filePath);
    void exportToODT(const QString &filePath);   

    void cut();
    void copy();
    void paste();
    void undo();
    void redo();
    void selectAll();

    void updatePasteAction();
    void updateConfiguration();
    void updateShortcut(int s, const QString &newShortcut);

    void openFilesAndShow(const QStringList &sl);

    void exportDirSlot();
private slots:
    void printFile();
    void printPreviewSlot();
    void showPreferencesDialog();
    void showAboutMdCharmDialog();
    void showSelectEncodingDialog();
    void showGotoDialog();
    void tabChange();
    void updatePreviewOptionActions(int type);
    void updateStatusBar();
    void checkUpdatesResult(const QString &info);
    void enableShowCongratulation();
    void openRecentFile();
    void openTheFile(const QString &filePath);
    void crateNewFile(const QString &dir);
    void updateRecentFileActions();
    void clearRecentFilesList();
    void findNextActionSlot();
    void findPreviousActionSlot();
    void addToRecentFileList(const QString &fileFullPath);
    void exportsActionSlot();
    void spellCheckActionSlot(bool checked);
    void showMarkdownCheatSheet();

    void markdownToolBarSlot();
    void previewOptionToolBarSlot();
signals:
    void updateRecentFileList();
    void mainWindowActived();
    void exportToOneFileFinished(bool isSuccess);
};

#endif // MDCHARMFORM_H
