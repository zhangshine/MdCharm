#include <cassert>

#include <QtGui>
#include <QApplication>
#include <QClipboard>

#include "mdcharmform.h"
#include "markdowneditareawidget.h"
#include "resource.h"
#include "markdowntohtml.h"
#include "utils.h"
#include "configuration.h"
#include "conf/configuredialog.h"
#include "about/aboutmdcharmdialog.h"
#include "version.h"
#include "utils.h"
#include "editareatabwidget.h"
#include "util/gui/addnewfiledialog.h"
#include "util/gui/selectencodingdialog.h"
#include "util/gui/gotodialog.h"
#include "util/gui/exportdialog.h"
#include "util/spellcheck/spellchecker.h"
#include "util/spellcheck/spellcheckselectordialog.h"
#include "util/sessionfileparser.h"
#include "util/gui/noticedialog.h"
#include "util/gui/markdowncheatsheetdialog.h"
#include "util/gui/statusbarlabel.h"
#include "util/rotationtoolbutton.h"
#include "util/gui/newversioninfodialog.h"
#include "util/gui/exportdirectorydialog.h"
#include "network/checkupdates.h"
#include "dock/projectdockwidget.h"
#include "dock/tocdockwidget.h"
#include "editareatabwidgetmanager.h"

MdCharmForm::MdCharmForm(QWidget *parent) :
    QMainWindow(parent)
{
    showCongratulation = false;
    clipboard = QApplication::clipboard();
    conf = Configuration::getInstance();
    mdcharmGlobalInstance = MdCharmGlobal::getInstance();
    cu = new CheckUpdates();
    mcsd = NULL;
    appTitle = QString::fromLatin1("MdCharm");
    initGui();
    initMenuContent();
    initToolBarContent();
    initStatusBarContent();
    initDockWidgets();
    initSignalsAndSlots();
    initShortcutMatters();

    updatePasteAction();

    connect(cu, SIGNAL(finished(QString)),
            this, SLOT(checkUpdatesResult(QString)));
    checkUpdatesTimer.setSingleShot(true);
    connect(&checkUpdatesTimer, SIGNAL(timeout()),
            cu, SLOT(check()));
    if(conf->isCheckForUpdates())
        checkUpdatesTimer.start(3000);//3s
}

MdCharmForm::~MdCharmForm()
{
    cu->deleteLater();
}

void MdCharmForm::initGui()
{
    setWindowIcon(QIcon(":/mdcharm.png"));
    setAcceptDrops(true);
    editAreaTabWidgetManager = new EditAreaTabWidgetManager(this);
    editAreaTabWidgetManager->setObjectName(QString::fromLatin1("editAreaTabWidgetManager"));
    setCentralWidget(editAreaTabWidgetManager);
    menuBar = new QMenuBar(this);
    menuBar->setObjectName(QString::fromLatin1("menuBar"));
    setMenuBar(menuBar);
    //Status Bar
    statusBar = new QStatusBar(this);
    statusBar->setObjectName(QString::fromLatin1("statusBar"));
    setStatusBar(statusBar);
    //Tool Bar
    toolBar = new QToolBar(tr("ToolBar"), this);
    toolBar->setObjectName(QString::fromLatin1("toolBar"));
    toolBar->setIconSize(QSize(16, 16));
    addToolBar(toolBar);
    markdownToolBar = new QToolBar(tr("Markdown ToolBar"), this);
    markdownToolBar->setObjectName("markdownToolBar");
    markdownToolBar->setIconSize(QSize(16, 16));
    markdownToolBar->setEnabled(false);
    addToolBar(markdownToolBar);
    previewOptionToolBar = new QToolBar(tr("Preview Option ToolBar"), this);
    previewOptionToolBar->setObjectName("previewOptionToolBar");;
    previewOptionToolBar->setIconSize(QSize(16, 16));;
    previewOptionToolBar->setEnabled(true);
    addToolBar(previewOptionToolBar);
    dockBar = new QToolBar(tr("Dock Bar"), this);
    dockBar->setObjectName("dockBar");
    dockBar->setIconSize(QSize(16, 16));
    dockBar->setAllowedAreas(Qt::LeftToolBarArea);
    dockBar->setFloatable(false);
    dockBar->setMovable(false);
    addToolBar(Qt::LeftToolBarArea, dockBar);
    resize(800, 600);
}

void MdCharmForm::initMenuContent()
{
    //File Menu
    fileMenu = new QMenu(tr("&File"), this);
    menuBar->addMenu(fileMenu);
    newAction = new QAction(QIcon(Resource::NewLargeIcon), tr("&New"), this);
    newAction->setData(MdCharmGlobal::ShortcutNew);
    newAction->setShortcut(mdcharmGlobalInstance->MdCharm_ShortCut_New);
    shortcutActions.append(newAction);
    fileMenu->addAction(newAction);
    openAction = new QAction(QIcon(Resource::OpenLargeIcon), tr("&Open..."), this);
    openAction->setToolTip(tr("Open a file"));
    openAction->setData(MdCharmGlobal::ShortcutOpen);
    openAction->setShortcut(mdcharmGlobalInstance->MdCharm_ShortCut_Open);
    shortcutActions.append(openAction);
    fileMenu->addAction(openAction);
    openDirectoryAction = new QAction(QIcon(Resource::OpenDirIcon),tr("Open Directory..."), this);
    openDirectoryAction->setToolTip(tr("Open a directory"));
    fileMenu->addAction(openDirectoryAction);
    saveAction = new QAction(QIcon(Resource::SaveLargeIcon), tr("&Save"), this);
    saveAction->setData(MdCharmGlobal::ShortcutSave);
    saveAction->setShortcut(mdcharmGlobalInstance->MdCharm_ShortCut_Save);
    saveAction->setEnabled(false);
    shortcutActions.append(saveAction);
    fileMenu->addAction(saveAction);
    saveAsAction = new QAction(QIcon(Resource::SaveAsLargeIcon), tr("Save &as..."), this);
    saveAsAction->setData(MdCharmGlobal::ShortcutSaveAs);
    saveAsAction->setShortcut(mdcharmGlobalInstance->MdCharm_ShortCut_SaveAs);
    saveAsAction->setEnabled(false);
    shortcutActions.append(saveAsAction);
    fileMenu->addAction(saveAsAction);   
    fileMenu->addSeparator();

    recentFileMenu = new QMenu(tr("Recent Files"), this);//recent file list menu
    fileMenu->addMenu(recentFileMenu);
    for(int i=0; i<MaxRecentFiles; i++)
    {
        recentFileActions[i] = new QAction(this);
        recentFileActions[i]->setVisible(false);
        connect(recentFileActions[i], SIGNAL(triggered()),
                this, SLOT(openRecentFile()));
        recentFileMenu->addAction(recentFileActions[i]);
    }
    recentFileMenu->addSeparator();
    clearRecentFilesAction = new QAction(tr("Clear Recent Files List"), this);
    recentFileMenu->addAction(clearRecentFilesAction);
    updateRecentFileActions();

    fileMenu->addSeparator();
    exportAction = new QAction(tr("&Export to..."), this);
    exportAction->setEnabled(false);
    fileMenu->addAction(exportAction);

    fileMenu->addSeparator();
    printPreviewAction = new QAction(tr("Print Pre&view..."), this);
    printPreviewAction->setShortcut(Qt::CTRL+Qt::SHIFT+Qt::Key_P);
    printPreviewAction->setEnabled(false);
    shortcutActions.append(printPreviewAction);
    fileMenu->addAction(printPreviewAction);
    printAction = new QAction(QIcon(Resource::PrintLargeIcon),tr("&Print..."), this);
    printAction->setData(MdCharmGlobal::ShortcutPrint);
    printAction->setShortcut(mdcharmGlobalInstance->MdCharm_ShortCut_Print);
    printAction->setEnabled(false);
    shortcutActions.append(printAction);
    fileMenu->addAction(printAction);

    fileMenu->addSeparator();
    quitAction = new QAction(tr("&Quit"), this);
    quitAction->setData(MdCharmGlobal::ShortcutQuit);
    quitAction->setShortcut(mdcharmGlobalInstance->MdCharm_ShortCut_Quit);
    shortcutActions.append(quitAction);
    fileMenu->addAction(quitAction);

    //Edit Menu
    editMenu = new QMenu(tr("&Edit"), this);
    menuBar->addMenu(editMenu);

    undoAction = new QAction(QIcon(Resource::UndoIcon), tr("&Undo"), this);
    undoAction->setData(MdCharmGlobal::ShortcutUndo);
    undoAction->setShortcut(mdcharmGlobalInstance->MdCharm_ShortCut_Undo);
    undoAction->setEnabled(false);
    shortcutActions.append(undoAction);
    editMenu->addAction(undoAction);
    redoAction = new QAction(QIcon(Resource::RedoIcon), tr("&Redo"), this);
    redoAction->setData(MdCharmGlobal::ShortcutRedo);
    redoAction->setShortcut(mdcharmGlobalInstance->MdCharm_ShortCut_Redo);
    redoAction->setEnabled(false);
    shortcutActions.append(redoAction);
    editMenu->addAction(redoAction);

    editMenu->addSeparator();

    cutAction = new QAction(QIcon(Resource::CutIcon), tr("Cu&t"), this);
    cutAction->setData(MdCharmGlobal::ShortcutCut);
    cutAction->setShortcut(mdcharmGlobalInstance->MdCharm_ShortCut_Cut);
    cutAction->setEnabled(false);
    shortcutActions.append(cutAction);
    editMenu->addAction(cutAction);
    copyAction = new QAction(QIcon(Resource::CopyIcon), tr("&Copy"), this);
    copyAction->setData(MdCharmGlobal::ShortcutCopy);
    copyAction->setShortcut(mdcharmGlobalInstance->MdCharm_ShortCut_Copy);
    copyAction->setEnabled(false);
    shortcutActions.append(copyAction);
    editMenu->addAction(copyAction);
    pasteAction = new QAction(QIcon(Resource::PasteIcon), tr("&Paste"), this);
    pasteAction->setData(MdCharmGlobal::ShortcutPaste);
    pasteAction->setShortcut(mdcharmGlobalInstance->MdCharm_ShortCut_Paste);
    pasteAction->setEnabled(false);
    shortcutActions.append(pasteAction);
    editMenu->addAction(pasteAction);

    editMenu->addSeparator();

    selectAllAction = new QAction(tr("&Select All"), this);
    editMenu->addAction(selectAllAction);
    selectAllAction->setData(MdCharmGlobal::ShortcutSelectAll);
    selectAllAction->setShortcut(mdcharmGlobalInstance->MdCharm_ShortCut_Select_All);
    selectAllAction->setEnabled(false);
    shortcutActions.append(selectAllAction);
    findMenu = new QMenu(tr("&Find"), this);
    editMenu->addMenu(findMenu);
    findAction = new QAction(QIcon(Resource::FindIcon), tr("Find..."), this);
    findAction->setData(MdCharmGlobal::ShortcutFind);
    findAction->setShortcut(mdcharmGlobalInstance->MdCharm_ShortCut_Find);
    shortcutActions.append(findAction);
    findMenu->addAction(findAction);
    findPreviousAction = new QAction(tr("Find Previous"), this);
    findPreviousAction->setData(MdCharmGlobal::ShortcutFindPrevious);
    findPreviousAction->setShortcut(mdcharmGlobalInstance->MdCharm_ShortCut_Find_Previous);
    shortcutActions.append(findPreviousAction);
    findMenu->addAction(findPreviousAction);
    findNextAction = new QAction(tr("Find Next"), this);
    findNextAction->setData(MdCharmGlobal::ShortcutFindNext);
    findNextAction->setShortcut(mdcharmGlobalInstance->MdCharm_ShortCut_Find_Next);
    shortcutActions.append(findNextAction);
    findMenu->addAction(findNextAction);
    //View Menu
    viewMenu = new QMenu(tr("&View"), this);
    menuBar->addMenu(viewMenu);
    //Tools Menu
    toolsMenu = new QMenu(tr("&Tools"), this);
//    menuBar->addMenu(toolsMenu); //TODO:
    exportDirAction = new QAction(tr("&Export Directory"), this);
//    toolsMenu->addAction(exportDirAction); //TODO:
    //Settings Menu
    settingsMenu = new QMenu(tr("&Settings"), this);
    menuBar->addMenu(settingsMenu);
    preferencesAction = new QAction(QIcon(Resource::ConfigIcon), tr("&Preferences..."), this);
    settingsMenu->addAction(preferencesAction);

    settingsMenu->addSeparator();

    writeModeAction = new QAction(QIcon(Resource::WriteModeIcon), tr("Show &Editor Only"), this);
    writeModeAction->setCheckable(true);
    writeModeAction->setData(MdCharmGlobal::WriteMode);
    writeModeAction->setIconVisibleInMenu(false);
    connect(writeModeAction, SIGNAL(triggered()), this, SLOT(previewOptionToolBarSlot()));
    settingsMenu->addAction(writeModeAction);
    writeReadAction = new QAction(QIcon(Resource::WriteReadIcon), tr("&Live Preview"), this);
    writeReadAction->setCheckable(true);
    writeReadAction->setData(MdCharmGlobal::WriteRead);
    writeReadAction->setIconVisibleInMenu(false);
    connect(writeReadAction, SIGNAL(triggered()), this, SLOT(previewOptionToolBarSlot()));
    settingsMenu->addAction(writeReadAction);
    readModeAction = new QAction(QIcon(Resource::ReadModeIcon), tr("Show Preview Panel &Only"), this);
    readModeAction->setCheckable(true);
    readModeAction->setData(MdCharmGlobal::ReadMode);
    readModeAction->setIconVisibleInMenu(false);
    connect(readModeAction, SIGNAL(triggered()), this, SLOT(previewOptionToolBarSlot()));
    settingsMenu->addAction(readModeAction);
    previewOptionActionGroup = new QActionGroup(this);
    previewOptionActionGroup->addAction(writeModeAction);
    previewOptionActionGroup->addAction(writeReadAction);
    previewOptionActionGroup->addAction(readModeAction);
    updatePreviewOptionActions(conf->getPreviewOption());

    settingsMenu->addSeparator();

    syncScrollbarAction = new QAction(tr("&Sync Scrollbar"), this);
    syncScrollbarAction->setCheckable(true);
    syncScrollbarAction->setChecked(conf->isSyncScrollbar());
    settingsMenu->addAction(syncScrollbarAction);
    //Help Menu
    helpMenu = new QMenu(tr("&Help"), this);
    menuBar->addMenu(helpMenu);
//    helpAction = new QAction(QIcon(Resource::HelpIcon), tr("Documentation"), this);
//    helpAction->setShortcut(QKeySequence::HelpContents);
//    helpMenu->addAction(helpAction);
//    helpMenu->addSeparator();
    markdownCheatSheetAction = new QAction(tr("Markdown Cheat Sheet"), this);
    markdownCheatSheetAction->setData(MdCharmGlobal::ShortcutCheatSheet);
    markdownCheatSheetAction->setShortcut(mdcharmGlobalInstance->MdCharm_ShortCut_Cheat_Sheet);
    shortcutActions.append(markdownCheatSheetAction);
    helpMenu->addAction(markdownCheatSheetAction);
    helpMenu->addSeparator();
    checkUpdatesAction = new QAction(tr("Check for Updates"), this);
    helpMenu->addAction(checkUpdatesAction);
    aboutAction = new QAction(tr("&About"), this);
    helpMenu->addAction(aboutAction);

    //Spell Check Action
    spellCheckAction = new QAction(QIcon(Resource::SpellCheckIcon), tr("Spell Check"), this);
    spellCheckAction->setCheckable(true);
    spellCheckAction->setChecked(conf->isCheckSpell());
}

void MdCharmForm::initStatusBarContent()
{
    lineAndColumnInfoLabel = new StatusBarLabel(statusBar);
    lineAndColumnInfoLabel->setToolTip(tr("Current line number and column number"));
    lineAndColumnInfoLabel->setFixedWidth(lineAndColumnInfoLabel->fontMetrics().width("-Line: XXXX Column: XXXX-"));
    connect(lineAndColumnInfoLabel, SIGNAL(labelClicked()),
            this, SLOT(showGotoDialog()));
    statusBar->addPermanentWidget(lineAndColumnInfoLabel);
    overWriteInfoLabel = new QLabel(statusBar);
    overWriteInfoLabel->setFixedWidth(overWriteInfoLabel->fontMetrics().width("-XXXXXX-"));//insert or over
    overWriteInfoLabel->setToolTip(tr("Editor insert mode"));
    statusBar->addPermanentWidget(overWriteInfoLabel);
    encodingInfoLabel = new StatusBarLabel(statusBar);
    encodingInfoLabel->setFixedWidth(encodingInfoLabel->fontMetrics().width("-XXXXXXX-XXXXXXX-"));//windows 1520
    encodingInfoLabel->setToolTip(tr("Current file encoding"));
    connect(encodingInfoLabel, SIGNAL(labelClicked()),
            this, SLOT(showSelectEncodingDialog()));
    statusBar->addPermanentWidget(encodingInfoLabel);
}

void MdCharmForm::initToolBarContent()
{
    viewMenu->addAction(toolBar->toggleViewAction());
    toolBar->addAction(newAction);
    toolBar->addAction(openAction);
    toolBar->addAction(saveAction);
    toolBar->addAction(saveAsAction);

    toolBar->addSeparator();

    toolBar->addAction(cutAction);
    toolBar->addAction(copyAction);
    toolBar->addAction(pasteAction);

    toolBar->addSeparator();

    toolBar->addAction(undoAction);
    toolBar->addAction(redoAction);

    toolBar->addSeparator();

    toolBar->addAction(spellCheckAction);
    //Markdown toolbar
    viewMenu->addAction(markdownToolBar->toggleViewAction());
    QAction *action;
    action = markdownToolBar->addAction(QIcon(Resource::ItalicIcon),
                                        tr("Italic(%1)").arg(mdcharmGlobalInstance->MdCharm_ShortCut_Italic.toString()),
                                        this, SLOT(markdownToolBarSlot()));
    action->setData(MdCharmGlobal::ShortcutItalic);
    action->setShortcut(mdcharmGlobalInstance->MdCharm_ShortCut_Italic);
    shortcutActions.append(action);

    action = markdownToolBar->addAction(QIcon(Resource::BoldIcon),
                                        tr("Bold(%1)").arg(mdcharmGlobalInstance->MdCharm_ShortCut_Bold.toString()),
                                        this, SLOT(markdownToolBarSlot()));
    action->setData(MdCharmGlobal::ShortcutBold);
    action->setShortcut(mdcharmGlobalInstance->MdCharm_ShortCut_Bold);
    shortcutActions.append(action);

    action = markdownToolBar->addAction(QIcon(Resource::QuoteIcon),
                                        tr("Quote(%1)").arg(mdcharmGlobalInstance->MdCharm_ShortCut_Quote_Text.toString()),
                                        this, SLOT(markdownToolBarSlot()));
    action->setData(MdCharmGlobal::ShortcutQuoteText);
    action->setShortcut(mdcharmGlobalInstance->MdCharm_ShortCut_Quote_Text);
    shortcutActions.append(action);

    strikeThoughAction = markdownToolBar->addAction(QIcon(Resource::StrikeThroughIcon),
                                        tr("Strike through(%1)").arg(mdcharmGlobalInstance->MdCharm_ShortCut_Strike_Through.toString()),
                                        this, SLOT(markdownToolBarSlot()));
    strikeThoughAction->setData(MdCharmGlobal::ShortcutStrikeThrough);
    strikeThoughAction->setEnabled(conf->getMarkdownEngineType()!=MarkdownToHtml::MultiMarkdown);
    strikeThoughAction->setShortcut(mdcharmGlobalInstance->MdCharm_ShortCut_Strike_Through);
    shortcutActions.append(action);

    action = markdownToolBar->addAction(QIcon(Resource::TabIcon), tr("Tab(Tab)"),
                                        this, SLOT(markdownToolBarSlot()));
    action->setData(MdCharmGlobal::ShortcutTabBlockText);

    action = markdownToolBar->addAction(QIcon(Resource::UntabIcon),
                                        tr("Untab(%1)").arg(mdcharmGlobalInstance->MdCharm_ShortCut_Shift_Tab.toString()),
                                        this, SLOT(markdownToolBarSlot()));
    action->setData(MdCharmGlobal::ShortcutShiftTab);
    action->setShortcut(mdcharmGlobalInstance->MdCharm_ShortCut_Shift_Tab);
    shortcutActions.append(action);

    action = markdownToolBar->addAction(QIcon(Resource::LinkIcon),
                                        tr("Insert Link(%1)").arg(mdcharmGlobalInstance->MdCharm_ShortCut_Insert_Link.toString()),
                                        this, SLOT(markdownToolBarSlot()));
    action->setData(MdCharmGlobal::ShortcutInsertLink);
    action->setShortcut(mdcharmGlobalInstance->MdCharm_ShortCut_Insert_Link);
    shortcutActions.append(action);

    action = markdownToolBar->addAction(QIcon(Resource::PictureIcon),
                                        tr("Insert Picture(%1)").arg(mdcharmGlobalInstance->MdCharm_ShortCut_Insert_Picture.toString()),
                                        this, SLOT(markdownToolBarSlot()));
    action->setData(MdCharmGlobal::ShortcutInsertPicture);
    action->setShortcut(mdcharmGlobalInstance->MdCharm_ShortCut_Insert_Picture);
    shortcutActions.append(action);

    action = markdownToolBar->addAction(QIcon(Resource::CodeIcon),
                                        tr("Insert Code(%1)").arg(mdcharmGlobalInstance->MdCharm_ShortCut_Insert_Code.toString()),
                                        this, SLOT(markdownToolBarSlot()));
    action->setData(MdCharmGlobal::ShortcutInsertCode);
    action->setShortcut(mdcharmGlobalInstance->MdCharm_ShortCut_Insert_Code);
    shortcutActions.append(action);

    //Preview Option ToolBar
    viewMenu->addAction(previewOptionToolBar->toggleViewAction());
    previewOptionToolBar->addAction(writeModeAction);
    previewOptionToolBar->addAction(writeReadAction);
    previewOptionToolBar->addAction(readModeAction);

    viewMenu->addAction(dockBar->toggleViewAction());
}

void MdCharmForm::initDockWidgets()
{
    //Project
    projectDockWidget = new ProjectDockWidget(this);
    addDockWidget(Qt::LeftDockWidgetArea, projectDockWidget);
    QAction *projectDockAction = projectDockWidget->toggleViewAction();
    projectDockAction->setShortcut(mdcharmGlobalInstance->MdCharm_ShortCut_Hide_Project_DockBar);
    projectDockAction->setData(MdCharmGlobal::ShortcutHideProjectDockBar);
    shortcutActions.append(projectDockAction);
    viewMenu->addSeparator();
    viewMenu->addAction(projectDockAction);

    projectDockWidget->setVisible(conf->isProjectDockWidgetVisible());

    RotationToolButton *btn = new RotationToolButton(dockBar);
    btn->setDefaultAction(projectDockAction);
    btn->setRotation(RotationToolButton::CounterClockwise);
    btn->setAutoRaise(true);
    dockBar->addWidget(btn);


    //Toc
    tocDockWidget = new TOCDockWidget(this);
    addDockWidget(Qt::LeftDockWidgetArea, tocDockWidget);

    QAction *tocDockAction = tocDockWidget->toggleViewAction();
    shortcutActions.append(tocDockAction);
    viewMenu->addAction(tocDockAction);

    tocDockWidget->setVisible(conf->isTocDockWidgetVisible());

    RotationToolButton *tocBtn = new RotationToolButton(dockBar);
    tocBtn->setDefaultAction(tocDockAction);
    tocBtn->setRotation(RotationToolButton::CounterClockwise);
    tocBtn->setAutoRaise(true);
    dockBar->addWidget(tocBtn);
}

void MdCharmForm::initSignalsAndSlots()
{
    connect(newAction, SIGNAL(triggered()), this, SLOT(addNewTabWidget()));
    connect(openAction, SIGNAL(triggered()), this, SLOT(openFiles()));
    connect(openDirectoryAction, SIGNAL(triggered()), this, SLOT(openDirectory()));
    connect(exportAction, SIGNAL(triggered()), this, SLOT(exportsActionSlot()));
    connect(saveAction, SIGNAL(triggered()), editAreaTabWidgetManager, SLOT(saveFile()));
    connect(saveAsAction, SIGNAL(triggered()), editAreaTabWidgetManager, SLOT(saveFileAs()));
    connect(this, SIGNAL(updateRecentFileList()), this, SLOT(updateRecentFileActions()));
    connect(printAction, SIGNAL(triggered()), this, SLOT(printFile()));
    connect(printPreviewAction, SIGNAL(triggered()), this, SLOT(printPreviewSlot()));
    connect(quitAction, SIGNAL(triggered()), this, SLOT(close()));
    connect(checkUpdatesAction, SIGNAL(triggered()), cu, SLOT(check()));
    connect(checkUpdatesAction, SIGNAL(triggered()), this, SLOT(enableShowCongratulation()));
    connect(aboutAction, SIGNAL(triggered()), this, SLOT(showAboutMdCharmDialog()));
    connect(editAreaTabWidgetManager, SIGNAL(currentChanged()), this, SLOT(tabChange()));
    connect(this, SIGNAL(mainWindowActived()), editAreaTabWidgetManager, SLOT(checkFileStatusWhenMainWindowActived()));
    connect(preferencesAction, SIGNAL(triggered()), this, SLOT(showPreferencesDialog()));
    if (clipboard)
        connect(clipboard, SIGNAL(dataChanged()), this, SLOT(updatePasteAction()));
    connect(pasteAction, SIGNAL(triggered()), this, SLOT(paste()));
    connect(cutAction, SIGNAL(triggered()), this, SLOT(cut()));
    connect(copyAction, SIGNAL(triggered()), this, SLOT(copy()));
    connect(selectAllAction, SIGNAL(triggered()), this, SLOT(selectAll()));
    connect(findAction, SIGNAL(triggered()), editAreaTabWidgetManager, SLOT(showFind()));
    connect(undoAction, SIGNAL(triggered()), this, SLOT(undo()));
    connect(redoAction, SIGNAL(triggered()), this, SLOT(redo()));
    connect(syncScrollbarAction, SIGNAL(toggled(bool)),
            editAreaTabWidgetManager, SLOT(changeSyncScrollBarSetting(bool)));
    connect(clearRecentFilesAction, SIGNAL(triggered()),
                     this, SLOT(clearRecentFilesList()));
    connect(projectDockWidget, SIGNAL(openTheFile(QString)), this, SLOT(openTheFile(QString)));
    connect(projectDockWidget, SIGNAL(createNewFile(QString)), this, SLOT(crateNewFile(QString)));
    connect(projectDockWidget, SIGNAL(deleteFileSignal(QString)),
                     editAreaTabWidgetManager, SLOT(deleteFileAndTab(QString)));
    connect(projectDockWidget, SIGNAL(renameFileSignal(QString,QString)),
                     editAreaTabWidgetManager, SLOT(renameFileAndTab(QString,QString)));
    connect(findNextAction, SIGNAL(triggered()), this, SLOT(findNextActionSlot()));
    connect(findPreviousAction, SIGNAL(triggered()), this, SLOT(findPreviousActionSlot()));
    connect(spellCheckAction, SIGNAL(triggered(bool)), this, SLOT(spellCheckActionSlot(bool)));
    connect(markdownCheatSheetAction, SIGNAL(triggered()), this, SLOT(showMarkdownCheatSheet()));
    connect(editAreaTabWidgetManager, SIGNAL(addToRecentFileList(QString)), this, SLOT(addToRecentFileList(QString)));
    connect(editAreaTabWidgetManager, SIGNAL(updateActions()), this, SLOT(updateActions()));
    connect(editAreaTabWidgetManager, SIGNAL(showStatusMessage(QString)), statusBar, SLOT(showMessage(QString)));
    connect(editAreaTabWidgetManager, SIGNAL(currentTabTextChanged()), this, SLOT(updateTocContent()));

    connect(exportDirAction, SIGNAL(triggered()), this, SLOT(exportDirSlot()));

    connect(tocDockWidget, SIGNAL(anchorClicked(QUrl)), this, SLOT(jumpToAnchor(QUrl)));
}

void MdCharmForm::initShortcutMatters()
{
    //FIXME: when use QKeySequence::Close, only Ctrl+F4 but not Ctrl+W works on
    //windows with Qt 4.8.1, so we use Ctrl+W instead of QKeySequence::Close.
    //When we developing for Mac platform, fix this
    closeTabShortcut = new QShortcut(mdcharmGlobalInstance->MdCharm_ShortCut_Close_Tab, this);
    closeTabShortcut->setContext(Qt::ApplicationShortcut);
    connect(closeTabShortcut, SIGNAL(activated()),
            editAreaTabWidgetManager, SLOT(closeCurrentTab()));
}

void MdCharmForm::setStatusMessage(QString &msg)
{
    statusBar->showMessage(msg);
}

EditAreaWidget* MdCharmForm::addNewTabWidget(const QString &filePath)
{
    return editAreaTabWidgetManager->addNewTabWidget(filePath);
}

void MdCharmForm::openFiles()
{
    QString selectedFilter;
    QStringList filterList = conf->getFileOpenFilters();
    if(conf->getLastFilterType()<filterList.length())
        selectedFilter = filterList.value(conf->getLastFilterType());
    QStringList filePaths = QFileDialog::getOpenFileNames(this,
                                                          tr("Select a file to open"),
                                                          conf->getLastOpenDir(),
                                                          filterList.join(";;"),
                                                          &selectedFilter);
    int currentIndex = filterList.indexOf(selectedFilter);
    if(currentIndex!=-1)
        conf->setLastFilterType(currentIndex);
    if (filePaths.isEmpty())
        return;
    QString path = filePaths.first();
    QFileInfo fileInfo(path);
    conf->setLastOpenDir(fileInfo.absolutePath());
    foreach (QString filePath, filePaths) {//open all
        addToRecentFileList(filePath);
        addNewTabWidget(filePath);
    }
}

void MdCharmForm::openDirectory()
{
    QString dir;
    QVariant var = conf->getLastStateValue("MdCharmForm_OpenDirectory");
    if(var.isValid() && var.canConvert(QVariant::String)){
        dir = var.toString();
    }
    QString dirPath = Utils::getExistingDirectory(this, tr("Select a directory to open"), dir);
    if(dirPath.isEmpty())
        return;
    conf->setLastStateValue("MdCharmForm_OpenDirectory", dirPath);
    projectDockWidget->setVisible(true);
    projectDockWidget->setProjectDir(dirPath);
}

void MdCharmForm::exportToHtml(const QString &filePath)
{
    if(filePath.isEmpty()){
        emit exportToOneFileFinished(false);
        return;
    }
    EditAreaWidget *editArea = editAreaTabWidgetManager->getCurrentWidget();
    assert(editArea);
    if(!editArea){
        emit exportToOneFileFinished(false);
        return;
    }
    editArea->exportToHtml(filePath);
    emit exportToOneFileFinished(true);
}

void MdCharmForm::exportToPdf(const QString &filePath)
{
    if (filePath.isEmpty()){
        emit exportToOneFileFinished(false);
        return;
    }
    EditAreaWidget *editArea = editAreaTabWidgetManager->getCurrentWidget();
    assert(editArea);
    if(!editArea){
        emit exportToOneFileFinished(false);
        return;
    }
    editArea->exportToPdf(filePath);
    emit exportToOneFileFinished(true);
}

void MdCharmForm::exportToODT(const QString &filePath)
{
    if (filePath.isEmpty()){
        emit exportToOneFileFinished(false);
        return;
    }
    EditAreaWidget *editArea = editAreaTabWidgetManager->getCurrentWidget();
    assert(editArea);
    if(!editArea){//null
        emit exportToOneFileFinished(false);
        return;
    }
    assert(editArea);
    editArea->exportToODT(filePath);
    emit exportToOneFileFinished(true);
}

void MdCharmForm::printFile()
{
    EditAreaWidget *editArea = editAreaTabWidgetManager->getCurrentWidget();
    assert(editArea);
    if(!editArea)
    {
        return;
    }
    editArea->printContent();
}

void MdCharmForm::printPreviewSlot()
{
    EditAreaWidget *editArea = editAreaTabWidgetManager->getCurrentWidget();
    assert(editArea);
    if(!editArea)
        return;
    editArea->printPreview();
}

// Settings Menu

void MdCharmForm::showPreferencesDialog()
{
    ConfigureDialog confDialog(this);
    QObject::connect(&confDialog, SIGNAL(updateConfiguration()),
                     this, SLOT(updateConfiguration()));
    QObject::connect(&confDialog, SIGNAL(updateShortcut(int,QString)),
                     this, SLOT(updateShortcut(int,QString)));
    if(confDialog.exec()==QDialog::Accepted)
        updateConfiguration();
}
// About Menu
void MdCharmForm::showAboutMdCharmDialog()
{
    AboutMdCharmDialog dialog(this);
    dialog.exec();
}

void MdCharmForm::updateActions()
{
    EditAreaWidget *editArea = editAreaTabWidgetManager->getCurrentWidget();
    if(!editArea) //null
    {
        cutAction->setEnabled(false);
        copyAction->setEnabled(false);
        pasteAction->setEnabled(false);
        undoAction->setEnabled(false);
        redoAction->setEnabled(false);
        selectAllAction->setEnabled(false);
        findAction->setEnabled(false);
        saveAction->setEnabled(false);
        saveAsAction->setEnabled(false);
        markdownToolBar->setEnabled(false);
        exportAction->setEnabled(false);
        printAction->setEnabled(false);
        printPreviewAction->setEnabled(false);
        setWindowTitle(appTitle);
        return;
    }
    assert(editArea);
    FileModel fm = editArea->getFileModel();
    EditorModel em = editArea->getEditorModel();
    if( em.getEditorType()==EditorModel::BROWER)
        updateWindowTitle(editAreaTabWidgetManager->getCurrentTabTabText(), false);
    else
        updateWindowTitle(fm.getFileFullPath(), editArea->isModified());
    if(em.getEditorType()==EditorModel::MARKDOWN)
        markdownToolBar->setEnabled(true);
    else
        markdownToolBar->setEnabled(false);
    //update actions
    updatePasteAction();
    updatePreviewOptionActions(conf->getPreviewOption());
    cutAction->setEnabled(em.isCopyAvailable());
    copyAction->setEnabled(em.isCopyAvailable());
    selectAllAction->setEnabled(editArea->isEditActionOptionEnabled(EditAreaWidget::AllowSelectAll));
    undoAction->setEnabled(editArea->isUndoAvailable());
    redoAction->setEnabled(editArea->isRedoAvailable());
    saveAsAction->setEnabled(editArea->isEditActionOptionEnabled(EditAreaWidget::AllowSaveAs));
    saveAction->setEnabled(editArea->isModified());
    exportAction->setEnabled( editArea->isEditActionOptionEnabled(EditAreaWidget::AllowExportToHtml) ||
                              editArea->isEditActionOptionEnabled(EditAreaWidget::AllowExportToODT) ||
                              editArea->isEditActionOptionEnabled(EditAreaWidget::AllowExportToPdf));
    printAction->setEnabled(editArea->isEditActionOptionEnabled(EditAreaWidget::AllowPrint));
    printPreviewAction->setEnabled(editArea->isEditActionOptionEnabled(EditAreaWidget::AllowPreview));
    if (syncScrollbarAction->isChecked() != conf->isSyncScrollbar())
        syncScrollbarAction->setChecked(conf->isSyncScrollbar());
    findAction->setEnabled(editArea->isEditActionOptionEnabled(EditAreaWidget::AllowFind));
    findNextAction->setEnabled(em.isFindVisible());
    findPreviousAction->setEnabled(em.isFindVisible());

    tocDockWidget->updateToc(conf->getMarkdownEngineType(), editArea->getText());
}

void MdCharmForm::updatePreviewOptionActions(int type)
{
    switch(type)
    {
        case MdCharmGlobal::WriteMode:
            if(!writeModeAction->isChecked())
                writeModeAction->setChecked(true);
            break;
        case MdCharmGlobal::WriteRead:
            if(!writeReadAction->isChecked())
                writeReadAction->setChecked(true);
            break;
        case MdCharmGlobal::ReadMode:
            if(!readModeAction->isChecked())
                readModeAction->setChecked(true);
            break;
        default:
            break;
    }
}

void MdCharmForm::updateStatusBar()
{
    EditAreaWidget *editArea = editAreaTabWidgetManager->getCurrentWidget();
    if(!editArea){
        lineAndColumnInfoLabel->clear();
        overWriteInfoLabel->clear();
        encodingInfoLabel->clear();
        return;
    }
    FileModel fm = editArea->getFileModel();
    EditorModel em = editArea->getEditorModel();
    if(!em.isEditable())// not editable
    {
        lineAndColumnInfoLabel->clear();
        overWriteInfoLabel->clear();
        encodingInfoLabel->clear();
        return;
    }
    lineAndColumnInfoLabel->setText(tr("Line: %1 Column: %2")
                                    .arg(em.getCurrentLineNumber())
                                    .arg(em.getCurrentColumnNumber()));
    overWriteInfoLabel->setText(em.isOverWrite()? tr("OVER") : tr("INSERT"));
    encodingInfoLabel->setText(fm.getEncodingFormatName() +
                               (fm.getEncodingFormatName().startsWith("UTF")? (fm.isHasBom() ? "" : " (No Bom)"):""));

}

void MdCharmForm::updateWindowTitle(const QString &title, bool isModified)
{
    QString formatTitle = title;
#ifdef Q_OS_WIN
    formatTitle.replace("/", "\\");
#endif
    setWindowModified(isModified);
    setWindowTitle(QString("%1[*] - %2")
                   .arg(title.isEmpty() ? editAreaTabWidgetManager->getCurrentTabTabText() : formatTitle)
                   .arg(appTitle));
}

void MdCharmForm::addToRecentFileList(const QString &fileFullPath)
{
    QString path = QFileInfo(fileFullPath).absoluteFilePath();
    QStringList rf = conf->getRecentFileList();
    int index = rf.indexOf(path);
    if(index!=-1) // not find
    {
        rf.removeAt(index);
    }
    rf.prepend(path);
    if(rf.length()>MaxRecentFiles)
    {
        rf.removeLast();
    }
    conf->setRecentFileList(rf);
    emit updateRecentFileList();
}

void MdCharmForm::updatePasteAction()
{
    if(clipboard)
    {
        QString text = clipboard->text();
        EditAreaWidget *editArea = editAreaTabWidgetManager->getCurrentWidget();
        if(!editArea)
            return;
        EditorModel em = editArea->getEditorModel();
        pasteAction->setEnabled(!text.isEmpty() && em.getEditorType()==EditorModel::MARKDOWN);
    }
}

void MdCharmForm::updateConfiguration()
{
    if(spellCheckAction->isChecked()!=conf->isCheckSpell())
        spellCheckAction->setChecked(conf->isCheckSpell());
    editAreaTabWidgetManager->updateConfiguration();
    if(conf->getMarkdownEngineType()==MarkdownToHtml::MultiMarkdown)
        strikeThoughAction->setEnabled(false);
    else
        strikeThoughAction->setEnabled(true);
}

void MdCharmForm::updateShortcut(int s, const QString &newShortcut)
{
    foreach (QAction *action, shortcutActions) {
        if(action->data().toInt()==s){
            action->setShortcut(newShortcut);
            QString actionText = action->text();
            int index = actionText.indexOf("(");
            if(index>0){
                actionText = actionText.left(index);
                action->setText(actionText+"("+newShortcut+")");
            }
            break;
        }
    }
}

void MdCharmForm::openFilesAndShow(const QStringList &sl)
{
    for(int i=0; i<sl.length(); i++){
        QString filePath = sl.at(i);
        if(filePath.isEmpty())
            continue;
        QString fullPath = QFileInfo(filePath).absoluteFilePath();
        addToRecentFileList(fullPath);
        addNewTabWidget(fullPath);
    }
    raise();
    activateWindow();
    setWindowState((windowState() & ~Qt::WindowMinimized) | (Qt::WindowActive));
}

void MdCharmForm::exportDirSlot()
{
    ExportDirectoryDialog edd(this);
    edd.exec();
}

void MdCharmForm::cut()
{
    EditAreaWidget *editArea = editAreaTabWidgetManager->getCurrentWidget();
    if(!editArea)
        return;
    assert(editArea);
    editArea->cut();
}

void MdCharmForm::copy()
{
    EditAreaWidget *editArea = editAreaTabWidgetManager->getCurrentWidget();
    if(!editArea)
        return;
    assert(editArea);
    editArea->copy();
}

void MdCharmForm::paste()
{
    EditAreaWidget *editArea = editAreaTabWidgetManager->getCurrentWidget();
    if(!editArea)
        return;
    assert(editArea);
    editArea->paste();
}

void MdCharmForm::undo()
{
    EditAreaWidget *editArea = editAreaTabWidgetManager->getCurrentWidget();
    if(!editArea)
        return;
    assert(editArea);
    editArea->undo();
}

void MdCharmForm::redo()
{
    EditAreaWidget *editArea = editAreaTabWidgetManager->getCurrentWidget();
    if(!editArea)
        return;
    assert(editArea);
    editArea->redo();
}

void MdCharmForm::selectAll()
{
    EditAreaWidget *editArea = editAreaTabWidgetManager->getCurrentWidget();
    if(!editArea)
        return;
    assert(editArea);
    editArea->selectAll();
}

void MdCharmForm::tabChange()
{
    EditAreaWidget *editArea = editAreaTabWidgetManager->getCurrentWidget();
    if(!editArea)//all tab closed
    {
        //disable copy cut paste selectAll
        updateActions();
        updateStatusBar();
        return;
    }
    disconnect(editArea, SIGNAL(updateActions()),
               this, SLOT(updateActions()));
    connect(editArea, SIGNAL(updateActions()),
            this, SLOT(updateActions()));
    updateActions();
    disconnect(editArea, SIGNAL(updateStatusBar()),
               this, SLOT(updateStatusBar()));
    connect(editArea, SIGNAL(updateStatusBar()),
            this, SLOT(updateStatusBar()));
    updateStatusBar();
}

void MdCharmForm::closeEvent(QCloseEvent *event)
{
    if(!editAreaTabWidgetManager->saveAllBeforeClose()){
        event->ignore();
        return;
    }
    //save window state
    saveMdCharmState();
    if(mcsd){
        mcsd->hide();
        mcsd->deleteLater();
    }
    QMainWindow::closeEvent(event);
}

void MdCharmForm::showSelectEncodingDialog()
{
    QString oldEncoding = encodingInfoLabel->text();
    if(oldEncoding.isEmpty())
        return;
    EditAreaWidget *eaw = editAreaTabWidgetManager->getCurrentWidget();
    if(!eaw)
        return;
    EditorModel em = eaw->getEditorModel();
    FileModel fm = eaw->getFileModel();
    SelectEncodingDialog sed(eaw->isModified(), fm.getEncodingFormatName(), this);
    if(sed.exec()==QDialog::Accepted && oldEncoding!=sed.getSelectedEncoding()
            && em.getEditorType()>=EditorModel::EDITABLE)
    {
        eaw->setFileEncoding(sed.getSelectedEncoding());
        encodingInfoLabel->setText(sed.getSelectedEncoding());
        if(sed.isReloadFile())
            eaw->reloadFile();
    }
}

void MdCharmForm::showGotoDialog()
{
    EditAreaWidget *eaw = editAreaTabWidgetManager->getCurrentWidget();
    if(!eaw)
        return;
    EditorModel em = eaw->getEditorModel();
    if(em.getEditorType()<EditorModel::EDITABLE)
        return;
    GotoDialog gd(eaw->getCurrentMaxBlockCount(), 1, this);
    if(gd.exec()==QDialog::Accepted)
    {
        eaw->gotoLine(gd.getLineNumber());
    }
}

void MdCharmForm::checkUpdatesResult(const QString &info)
{
    QStringList versionInfoList = info.split('|');
    if(versionInfoList.length()<5)
        return;
    bool isNew = false;
    bool isRecommand = false;
    QString warning;
    QString version;
    QString revision;
    for( int i=0; i<versionInfoList.length(); i++)
    {
        QString v = versionInfoList.at(i);
        if(v.startsWith(QString::fromLatin1("revision="))){
            if(v!=QString::fromLatin1("revision=%1").arg(REVISION_STR))
                isNew = true;
            else
                isNew = false;
            revision = v.right(v.length()-v.lastIndexOf('=')-1);
            continue;
        }
        if(v.startsWith(QString::fromLatin1("isrecommand=0"))){
            isRecommand = false;
            continue;
        }
        if(v.startsWith(QString::fromLatin1("isrecommand=1"))){
            isRecommand = true;
            continue;
        }
        if(v.startsWith(QString::fromLatin1("version="))){
            if(v.length()>=9)
                version= v.right(v.length()-v.lastIndexOf('=')-1);
            continue;
        }
        if(v.startsWith(QString::fromLatin1("warning="))){
            if(v.length()>=9)
                warning = v.right(v.length()-v.indexOf('=')-1);
            continue;
        }
    }
    if(isNew && (isRecommand||showCongratulation) && !version.isEmpty() && revision!=conf->getLastIgnoreRevision()){
        NewVersionInfoDialog nvid(version, revision, warning, this);
        nvid.exec();
    } else {
        if(showCongratulation)
            QMessageBox::information(this, tr("Congratulations!"), tr("No new release found!"));
    }
}

void MdCharmForm::enableShowCongratulation()
{
    showCongratulation = true;
}

void MdCharmForm::openRecentFile()
{
    QAction *act = qobject_cast<QAction *>(sender());
    if(act)
    {
        openTheFile(act->data().toString());
    }
}

void MdCharmForm::openTheFile(const QString &filePath)
{
    QFileInfo fileInfo(filePath);
    if(fileInfo.exists())
        addToRecentFileList(fileInfo.absoluteFilePath());
    else //file not exist
    {
        if(QMessageBox::warning(this, tr("File not exist"),
                                tr("%1 doesn't' exist, do you want to create it?").arg(fileInfo.absoluteFilePath()),
                                QMessageBox::Yes|QMessageBox::No)
                ==QMessageBox::Yes)
        {
            fileInfo.absoluteDir().mkpath(fileInfo.absoluteDir().absolutePath());
            QFile tryCreate(fileInfo.absoluteFilePath());
            if(!tryCreate.open(QIODevice::WriteOnly))
            {
                Utils::showFileError(tryCreate.error(), fileInfo.absoluteFilePath());
                return;
            }
            tryCreate.close();
            addToRecentFileList(fileInfo.absoluteFilePath());
        } else {
            conf->removeFromRecentFileList(fileInfo.absoluteFilePath().replace("\\","/"));
            updateRecentFileList();
            return;
        }
    }
    addNewTabWidget(fileInfo.absoluteFilePath());
}

void MdCharmForm::crateNewFile(const QString &dir)
{
    AddNewFileDialog anfd(this);
    anfd.setParentDir(dir);
    int res = anfd.exec();
    if(res==QDialog::Accepted)
    {
        QString realName = Utils::checkOrAppendDefaultSuffix(MdCharmGlobal::Markdown, anfd.getFileName());
        QString realPath = dir + "/" + realName;
        QFile newFile(realPath);
        if(!newFile.open(QIODevice::WriteOnly))
            return Utils::showFileError(newFile.error(), realPath);
        newFile.close();
        openTheFile(realPath);
    }
}

void MdCharmForm::updateRecentFileActions()
{
    QStringList rf = conf->getRecentFileList();
    if(rf.isEmpty())
    {
        recentFileMenu->setEnabled(false);
        return;
    } else {
        recentFileMenu->setEnabled(true);
    }
    int len = rf.length()<=MaxRecentFiles ? rf.length() : MaxRecentFiles;
    for (int i=0; i<len; i++)
    {
        QAction *act = recentFileActions[i];
        QString path = rf.at(i);
        act->setData(path);
#ifdef Q_OS_WIN
        path.replace('/','\\');//keep this
#endif
        act->setText(path);//replace method will change the string
        act->setVisible(true);
    }
    for(int i=len; i<MaxRecentFiles; i++)
    {
        recentFileActions[i]->setVisible(false);
    }
}

void MdCharmForm::restoreMdCharmState()
{
    restoreGeometry(conf->getGeometryState());
    restoreState(conf->getWindowState());
    SessionFileParser sfp;
    if( !sfp.startParse(conf->getSessionFilePath()) ){
        return;
    }
    projectDockWidget->setProjectDir(sfp.getProjectDockWidgetDirPath());//must restor project dir first
    editAreaTabWidgetManager->restoreTabsState(sfp.getFileStateModelList());
}

void MdCharmForm::saveMdCharmState()
{
    conf->setGeometryState(saveGeometry());
    conf->setWindowState(saveState());
    editAreaTabWidgetManager->saveTabsState();
}

void MdCharmForm::openArgFiles(QStringList &fileList)
{
    if(fileList.isEmpty())
        return;
    for(int i=0; i<fileList.size(); i++)
    {
        QString fullPath = QFileInfo(fileList.at(i)).absoluteFilePath();
        addToRecentFileList(fullPath);
        addNewTabWidget(fullPath);
    }
}

void MdCharmForm::checkCodeSyntaxCss()
{
    if(conf->isUseMarkdownDefaultCSS()){
        conf->setAppendCodeSyntaxCss(true);
    } else if(!conf->isAppendCodeSyntaxCss()) { //user custom css
        QFile codeSyntaxPatchFile(":/markdown/code_syntax_patch.css");
        if(!codeSyntaxPatchFile.open(QFile::ReadOnly))
            return;
        QTextStream stream(&codeSyntaxPatchFile);
        conf->setMarkdownCSS(false, conf->getMarkdownCSS()+stream.readAll());
        codeSyntaxPatchFile.close();
        conf->setAppendCodeSyntaxCss(true);
        QFile codeSyntaxNoticeFile(":/markdown/code_syntax_notice.html");
        if(!codeSyntaxNoticeFile.open(QFile::ReadOnly))
            return;
        QString notice = codeSyntaxNoticeFile.readAll();
        codeSyntaxNoticeFile.close();
        showNotice(notice);
    }
}

ProjectDockWidget* MdCharmForm::getProjectDockWidget()
{
    return projectDockWidget;
}

void MdCharmForm::showNotice(const QString &notice)
{
    QMessageBox::information(this, tr("Notice"), notice, QMessageBox::Ok);
}

void MdCharmForm::clearRecentFilesList()
{
    conf->setRecentFileList(QStringList());
    updateRecentFileActions();
    emit updateRecentFileList();
}

void MdCharmForm::findNextActionSlot()
{
    EditAreaWidget *eaw = editAreaTabWidgetManager->getCurrentWidget();
    if(!eaw)
        return;
    eaw->findNext();
}

void MdCharmForm::findPreviousActionSlot()
{
    EditAreaWidget *eaw = editAreaTabWidgetManager->getCurrentWidget();
    if(!eaw)
        return;
    eaw->findPrevious();
}

void MdCharmForm::exportsActionSlot()
{
    ExportDialog ed(this);
    connect(&ed, SIGNAL(exportToHtml(QString)),
            this, SLOT(exportToHtml(QString)));
    connect(&ed, SIGNAL(exportToOdt(QString)),
            this, SLOT(exportToODT(QString)));
    connect(&ed, SIGNAL(exportToPdf(QString)),
            this, SLOT(exportToPdf(QString)));
    connect(this, SIGNAL(exportToOneFileFinished(bool)),
            &ed, SLOT(exportOneFileFinished(bool)));
    ed.exec();
}

void MdCharmForm::spellCheckActionSlot(bool checked)
{
    conf->setCheckSpell(checked);
    if(conf->getSpellCheckLanguage().isEmpty()){
        SpellCheckSelectorDialog scsd(this);
        if(scsd.exec()==QDialog::Accepted)
            conf->setSpellCheckLanguage(scsd.getSpellCheckLanguageName());
    }
    if(conf->getSpellCheckLanguage().isEmpty()){
        conf->setCheckSpell(false);
    }
    editAreaTabWidgetManager->updateSpellCheckOption(checked);
    if(conf->isCheckSpell()!=spellCheckAction->isChecked()){
        disconnect(spellCheckAction, SIGNAL(triggered(bool)), this, SLOT(spellCheckActionSlot(bool)));
        spellCheckAction->setChecked(conf->isCheckSpell());
        connect(spellCheckAction, SIGNAL(triggered(bool)), this, SLOT(spellCheckActionSlot(bool)));
    }
}

void MdCharmForm::showMarkdownCheatSheet()
{
    if(!mcsd)
        mcsd = new MarkdownCheatSheetDialog;
    mcsd->showAndPopup();
}

//---------------------------- Drag and Drop -----------------------------------
void MdCharmForm::dragEnterEvent(QDragEnterEvent *e)
{
    if(e->mimeData()->hasFormat("text/plain")||
            e->mimeData()->hasFormat("text/uri-list"))
    {
        e->acceptProposedAction();
    }
}

void MdCharmForm::dragLeaveEvent(QDragLeaveEvent *e)
{
    Q_UNUSED(e)
}

void MdCharmForm::dropEvent(QDropEvent *e)
{
    const QMimeData* mimeData = e->mimeData();
    if (mimeData->hasUrls())
    {
        QList<QUrl> urlList = mimeData->urls();
        for(int i=0; i<urlList.size() && i<32; i++)
        {
            openTheFile(urlList.at(i).toLocalFile());
        }
    } else if (mimeData->hasText()){
        //TODO: current it only deal text as markdown
        EditAreaWidget * eaw = addNewTabWidget();
        if(eaw)
            eaw->setText(mimeData->text());
    }
}

//------------------- Change Event ---------------------------------------------
void MdCharmForm::changeEvent(QEvent *e)
{
    QMainWindow::changeEvent(e);
    if (e->type() == QEvent::ActivationChange) {
        if (isActiveWindow()) {
            emit mainWindowActived();
        }
    }
}

//------------------- Markdown ToolBar -----------------------------------------
void MdCharmForm::markdownToolBarSlot()
{
    if(conf->getPreviewOption()==MdCharmGlobal::ReadMode)//in reader mode, ignore markdown menu actions
        return;
    QAction *action = qobject_cast<QAction *>(sender());
    if(!action)
        return;
    EditAreaWidget *editArea = editAreaTabWidgetManager->getCurrentWidget();
    if(!editArea)
        return;
    EditorModel em = editArea->getEditorModel();
    if(em.getEditorType()!=EditorModel::MARKDOWN)
        return;
    MarkdownEditAreaWidget *meaw = qobject_cast<MarkdownEditAreaWidget *>(editArea);
    if(!meaw)
        return;
    meaw->dealMarkdownMenuAction(action->data().toInt());
}

//-----------------------Preview Option ToolBar---------------------------------
void MdCharmForm::previewOptionToolBarSlot()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if(!action)
        return;
    conf->setPreviewOption(action->data().toInt());
    editAreaTabWidgetManager->switchPreview(action->data().toInt());
}

//--------------------- Toc ----------------------------------------------------
void MdCharmForm::updateTocContent()
{
    EditAreaWidget *editArea = editAreaTabWidgetManager->getCurrentWidget();
    if(!editArea)
        return;
    tocDockWidget->updateToc(conf->getMarkdownEngineType(), editArea->getText());
}

void MdCharmForm::jumpToAnchor(const QUrl &url)
{
    EditAreaWidget *editArea = editAreaTabWidgetManager->getCurrentWidget();
    if(!editArea)
        return;
    MarkdownEditAreaWidget *meaw = qobject_cast<MarkdownEditAreaWidget *>(editArea);
    if(!meaw)
        return;
    meaw->jumpToPreviewAnchor(url.fragment());
}
