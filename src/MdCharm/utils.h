#ifndef UTILS_H
#define UTILS_H

#include <QString>
#include <QFile>
#include <QFileDialog>
#include <QKeySequence>
#include <QIcon>
#include <QMap>
#include <QWidget>

#include "markdowntohtml.h"

class SpellChecker;
class Configuration;

class MdCharmGlobal : public QWidget
{
    Q_OBJECT
public:
    enum SaveFileOptions
    {
        Save,
        DontSave,
        Cancel,
        None
    };
    enum UpdatesInfo
    {
        Version=0,
        Revision,
        UpdateXmlUrl,
        IsRecommand,
        Warning
    };
    enum WikiType
    {
        Markdown
    };
    enum UTF8BOM
    {
        Add,
        Keep,
        Delete
    };

    enum PreviewOptionMenu
    {
        PreviewOptionMenu_WriteMode,
        PreviewOptionMenu_WriteRead,
        PreviewOptionMenu_ReadMode
    };
    enum PreviewOption
    {
        WriteMode,
        WriteRead,
        ReadMode
    };

    enum Shortcuts
    {
        //Warning: Don't change the order
        ShortcutCloseTab,
        ShortcutItalic,
        ShortcutBold,
        ShortcutQuoteText,
        ShortcutShiftTab,
        ShortcutStrikeThrough,
        ShortcutInsertLink,
        ShortcutInsertPicture,
        ShortcutCheatSheet,
        ShortcutInsertCode,

        ShortcutNew,
        ShortcutOpen,
        ShortcutSave,
        ShortcutSaveAs,
        ShortcutPrint,
        ShortcutQuit,
        ShortcutUndo,
        ShortcutRedo,
        ShortcutCut,
        ShortcutCopy,
        ShortcutPaste,
        ShortcutSelectAll,
        ShortcutFind,
        ShortcutFindPrevious,
        ShortcutFindNext,
        ShortcutPrintPreview,

        ShortcutHideProjectDockBar,

        ShortcutEnd,

        ShortcutHelpContents,
        ShortcutTabBlockText//not real a shortcut
    };

    QKeySequence MdCharm_ShortCut_Close_Tab;
    QKeySequence MdCharm_ShortCut_Italic;
    QKeySequence MdCharm_ShortCut_Bold;
    QKeySequence MdCharm_ShortCut_Quote_Text;
    QKeySequence MdCharm_ShortCut_Shift_Tab;
    QKeySequence MdCharm_ShortCut_Strike_Through;
    QKeySequence MdCharm_ShortCut_Insert_Link;
    QKeySequence MdCharm_ShortCut_Insert_Picture;
    QKeySequence MdCharm_ShortCut_Cheat_Sheet;
    QKeySequence MdCharm_ShortCut_Insert_Code;
    QKeySequence MdCharm_ShortCut_New;
    QKeySequence MdCharm_ShortCut_Open;
    QKeySequence MdCharm_ShortCut_Save;
    QKeySequence MdCharm_ShortCut_SaveAs;
    QKeySequence MdCharm_ShortCut_Print;
    QKeySequence MdCharm_ShortCut_Quit;
    QKeySequence MdCharm_ShortCut_Undo;
    QKeySequence MdCharm_ShortCut_Redo;
    QKeySequence MdCharm_ShortCut_Cut;
    QKeySequence MdCharm_ShortCut_Copy;
    QKeySequence MdCharm_ShortCut_Paste;
    QKeySequence MdCharm_ShortCut_Select_All;
    QKeySequence MdCharm_ShortCut_Find;
    QKeySequence MdCharm_ShortCut_Find_Previous;
    QKeySequence MdCharm_ShortCut_Find_Next;
    QKeySequence MdCharm_ShortCut_Help_Contents;
    QKeySequence MdCharm_ShortCut_Print_Preview;
    QKeySequence MdCharm_ShortCut_Hide_Project_DockBar;

private:
    static MdCharmGlobal *instance;
    Configuration *conf;
    QMap<QString, SpellChecker*> spellCheckerManager;
    QStringList spellCheckerLanguageList;
    QMap<QString, QString> cacheSpellCheckDictLocaleName;
private:
    MdCharmGlobal();
    ~MdCharmGlobal();
    bool loadSpellCheck(const QString &lan);
public:
    static MdCharmGlobal *getInstance();
    SpellChecker* getSpellChecker(const QString &lan);
    QStringList getSpellCheckerLanguageList();
    QString getDictLocaleName(const QString &dictName);
    QString getShortDescriptionText(int s);
};

class Utils
{
public:
    static QString getSaveFileName(const QString &suffix, QWidget *parent=0,
                                   const QString &caption=QString(),
                                   const QString &dir = QString(),
                                   const QString &filter=QString(),
                                   QString *selectedFilter=0);
    static QString getExistingDirectory(QWidget* parent=0, const QString &caption=QString(),
                                        const QString &dir=QString(),
                                        QFileDialog::Options options= QFileDialog::ShowDirsOnly);
    static bool saveFile(const QString &fileFullPath, const QString &content);
    static bool saveFile(const QString &fileFullPath, const QByteArray &content);
    static void showFileError(QFile::FileError error, const QString &filePath);
    static QByteArray encodeString(const QString src, const QString &codecName, bool addBom=false);
    static QString checkOrAppendDefaultSuffix(MdCharmGlobal::WikiType type, const QString &fileName);
    static QStringList getEncodingList();
    static bool isUtf8WithoutBom(const QByteArray &content);
    static bool isMarkdownFile(const QString &fileName);
    static QFileInfoList listAllFileInDir(const QString &filePath,
                                          const QStringList &nameFilter,
                                          QDir::Filters filters=QDir::NoFilter,
                                          bool isListSubdir = false);
    static bool calculateBom(bool hasBom, const QString &encoding, MdCharmGlobal::UTF8BOM ub);
    static bool isLetterOrNumberString(const QString &str);
    static void showInGraphicalShell(const QString &path);
    static QString translateMarkdown2Html(MarkdownToHtml::MarkdownType type, const QString &content);
    static QString readFile(const QString &filePath);
    static QString getHtmlTemplate();

    static QStringList ImageExts;
public:
    static QString AppName;
    static const char* AppNameCStr;
};

#endif // UTILS_H
