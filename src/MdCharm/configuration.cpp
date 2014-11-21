#include <QSettings>
#include <QDir>
#include <QApplication>
#include <QTextStream>
#include <QtCore>
#include <QCoreApplication>

#include "configuration.h"
#include "version.h"
#include "utils.h"

namespace {
static const char *SPELL_CHECK_DIC_DIRECTORY_NAME = "spellcheckdict";

#if defined(Q_WS_MAC)
    enum { DEFAULT_FONT_SIZE = 12 };
    static const char *DEFAULT_FONT_FAMILY = "Monaco";
#elif defined(Q_WS_X11)
    enum { DEFAULT_FONT_SIZE = 9 };
    static const char *DEFAULT_FONT_FAMILY = "Monospace";
#else
    enum { DEFAULT_FONT_SIZE = 10 };
    static const char *DEFAULT_FONT_FAMILY = "Courier";
#endif
}

Configuration *Configuration::instance = 0;
const QString Configuration::FONT_FAMILY = QString::fromLatin1("Common/FontFamily");
const QString Configuration::FONT_SIZE = QString::fromLatin1("Common/FontSize");
const QString Configuration::TAB_SIZE = QString::fromLatin1("TextEditor/TabSize");
const QString Configuration::DISPLAY_LINE_NUMBER = QString::fromLatin1("TextEditor/DisplayLineNumber");
const QString Configuration::HIGHLIGHT_CURRENT_LINE = QString::fromLatin1("TextEditor/HighlighCurrentLine");
const QString Configuration::ENABLE_TEXT_WRAPPING = QString::fromLatin1("TextEditor/EnableTextWrapping");
const QString Configuration::PREVIEW_OPTION = QString::fromLatin1("Behavior/PreviewOption");
const QString Configuration::IS_SHOW_SPLASH = QString::fromLatin1("Common/ShowSplash");
const QString Configuration::IS_CHECK_FOR_UPDATES = QString::fromLatin1("Common/CheckForUpdates");
const QString Configuration::RECENT_FILE_LIST = QString::fromLatin1("Common/RecentFiles");
const QString Configuration::MARKDOWN_USE_DEFAULT_CSS = QString::fromLatin1("Styles/MarkdownDefaultCSSPath");
const QString Configuration::IS_PROJECT_DOCK_WIDGET_VISIBLE = QString::fromLatin1("Dock/ProjectDockWidgetVisible");
const QString Configuration::IS_TOC_DOCK_WIDGET_VISIBLE = QString::fromLatin1("Dock/TocDockWidgetVisible");
const QString Configuration::DEFAULT_ENCODING = QString::fromLatin1("TextEditor/DefaultEncoding");
const QString Configuration::UTF8_BOM = QString::fromLatin1("TextEditor/Utf8Bom");
const QString Configuration::SYNC_SCROLLBAR = QString::fromLatin1("Behavior/SyncScrollbar");
const QString Configuration::IS_ENABLE_SPELL_CHECK = QString::fromLatin1("TextEditor/SpellCheck");
const QString Configuration::SPELL_CHECK_LANGUAGE = QString::fromLatin1("TextEditor/SpellCheckLanguage");
const QString Configuration::USE_WHITE_SPACE_INSTEAD_OF_TAB = QString::fromLatin1("TextEditor/UseWhiteSpaceInsteadOfTab");
const QString Configuration::GEOMETRY_STATE = QString::fromLatin1("MdCharmState/GeometeryState");
const QString Configuration::WINDOW_STATE = QString::fromLatin1("MdCharmState/WindowState");
const QString Configuration::APPEND_CODE_SYNTAX_CSS = QString::fromLatin1("MdCharmUpdate/AppendCodeSyntaxCss");
const QString Configuration::LAST_OPEN_DIR = QString::fromLatin1("MdCharmState/LastOpenDir");
const QString Configuration::HIDE_FILE_EXT_IN_PROJECT_DOCK = QString::fromLatin1("MdCharmState/HideFileExtInProjectDock");
const QString Configuration::LAST_FILTER_TYPE = QString::fromLatin1("MdCharmState/LastFilterType");//Same as the FileType enum
const QString Configuration::FILE_EXTENSION = QString::fromLatin1("Common/FileExtension");
const QString Configuration::AUTO_INDENTATION = QString::fromLatin1("TextEditor/AutoIndentation");
const QString Configuration::AUTO_PAIR = QString::fromLatin1("TextEditor/AutoPair");
const QString Configuration::LAST_IGNORE_REVISION = QString::fromLatin1("Update/LastIgnoreRevision");
const QString Configuration::RIGHT_MARGIN_COLUMN = QString::fromLatin1("TextEditor/RightMarginColumn");
const QString Configuration::MARKDOWN_ENGINE = QString::fromLatin1("Common/MarkdownEngine");
const QString Configuration::LAST_STATE_GROUP = QString::fromLatin1("LastState/");
const QString Configuration::SHORTCUTS_GROUP = QString::fromLatin1("Shortcuts/");

Configuration::Configuration()
{
    settings = new QSettings(QSettings::IniFormat, QSettings::UserScope,
                             Utils::AppName, Utils::AppName);
    regSettings = new QSettings(QSettings::NativeFormat, QSettings::UserScope,
                                Utils::AppName, Utils::AppName);
#if defined(Q_OS_WIN)
    defaultSpellCheckRootDir = QCoreApplication::applicationDirPath();
#elif defined(Q_OS_LINUX)
    defaultSpellCheckRootDir = "/usr/share/mdcharm";//linux
#else
    defaultSpellCheckRootDir = configFileDirPath();//mac
#endif
    writeSystemInfo();

    //Must Order By FileType
    fileTypeString
            << tr("All Files") //All
            << tr("Markdown"); //Markdown
    fileTypeSuffixString
            << "*.*"                        //All
            << "*.markdown|*.md|*.mkd";     //Markdown
}

Configuration::~Configuration()
{
    settings->sync();
}

void Configuration::setFontFamily(const QString &family)
{
    settings->setValue(FONT_FAMILY, family);
}

QString Configuration::getFontFamily()
{
    QVariant var = settings->value(FONT_FAMILY);
    if (var.isValid() && var.canConvert(QVariant::String))
    {
        return var.toString();
    } else {
        QString r(DEFAULT_FONT_FAMILY);
        setFontFamily(r);
        return r;
    }
}

void Configuration::setFontSize(const int fontSize)
{
    settings->setValue(FONT_SIZE, fontSize);
}

int Configuration::getFontSize()
{
    QVariant var = settings->value(FONT_SIZE);
    if(var.isValid() && var.canConvert(QVariant::Int))
    {
        return var.toInt();
    } else {
        setFontSize(DEFAULT_FONT_SIZE);
        return DEFAULT_FONT_SIZE;
    }
}

void Configuration::setTabSize(const int tabSize)
{
    settings->setValue(TAB_SIZE, tabSize);
}

int Configuration::getTabSize()
{
    QVariant var = settings->value(TAB_SIZE);
    if(var.isValid() && var.canConvert(QVariant::Int))
    {
        return var.toInt();
    } else {
        setTabSize(4);
        return 4;
    }
}

void Configuration::setDisplayLineNumber(const bool b)
{
    settings->setValue(DISPLAY_LINE_NUMBER, b);
}

bool Configuration::isDisplayLineNumber()
{
    QVariant var = settings->value(DISPLAY_LINE_NUMBER);
    if (var.isValid() && var.canConvert(QVariant::Bool))
    {
        return var.toBool();
    } else {
        setDisplayLineNumber(false);
        return false;
    }
}

void Configuration::setEnableTextWrapping(const bool b)
{
    settings->setValue(ENABLE_TEXT_WRAPPING, b);
}

bool Configuration::isEnableTextWrapping()
{
    QVariant var = settings->value(ENABLE_TEXT_WRAPPING);
    if (var.isValid() && var.canConvert(QVariant::Bool))
    {
        return var.toBool();
    } else {
        setEnableTextWrapping(true);
        return true;
    }
}

void Configuration::setHighlightCurrentLine(const bool b)
{
    settings->setValue(HIGHLIGHT_CURRENT_LINE, b);
}

bool Configuration::isHighlightCurrentLine()
{
    QVariant var = settings->value(HIGHLIGHT_CURRENT_LINE);
    if (var.isValid() && var.canConvert(QVariant::Bool))
    {
        return var.toBool();
    } else {
        setHighlightCurrentLine(false);
        return false;
    }
}

void Configuration::setPreviewOption(const int b)
{
    settings->setValue(PREVIEW_OPTION, b);
}

int Configuration::getPreviewOption()
{
    QVariant var = settings->value(PREVIEW_OPTION);
    if (var.isValid() && var.canConvert(QVariant::Int))
    {
        return var.toInt();
    } else {
        if(isShowPreview_Old()){
            setPreviewOption(MdCharmGlobal::WriteRead);
            return MdCharmGlobal::WriteRead;
        } else {
            setPreviewOption(MdCharmGlobal::WriteMode);
            return MdCharmGlobal::WriteMode;
        }
    }
}
//This method is used for compatible with version before 0.9.8
//TODO: remove it someday
bool Configuration::isShowPreview_Old()
{
    QVariant var = settings->value(QString::fromLatin1("Behavior/Preview"));
    if(var.isValid() && var.canConvert(QVariant::Bool)){
        return var.toBool();
    } else {
        return true;
    }
}

void Configuration::changeSyncScrollbarSetting(bool sync)
{
    settings->setValue(SYNC_SCROLLBAR, sync);
}

bool Configuration::isSyncScrollbar()
{
    QVariant var = settings->value(SYNC_SCROLLBAR);
    if (var.isValid() && var.canConvert(QVariant::Bool))
    {
        return var.toBool();
    } else {
        changeSyncScrollbarSetting(true);
        return true;
    }
}

void Configuration::setCheckForUpdates(const bool b)
{
    settings->setValue(IS_CHECK_FOR_UPDATES, b);
}

bool Configuration::isCheckForUpdates()
{
    QVariant var = settings->value(IS_CHECK_FOR_UPDATES);
    if (var.isValid() && var.canConvert(QVariant::Bool))
    {
        return var.toBool();
    } else {
        setCheckForUpdates(true);
        return true;
    }
}

void Configuration::setShowSplash(const bool b)
{
    settings->setValue(IS_SHOW_SPLASH, b);
}

bool Configuration::isShowSplash()
{
    QVariant var = settings->value(IS_SHOW_SPLASH);
    if(var.isValid() && var.canConvert(QVariant::Bool))
        return var.toBool();
    else
    {
#if Q_OS_WIN
        setShowSplash(true);
        return true;
#else
        setShowSplash(false);
        return false;
#endif
    }
}

Configuration* Configuration::getInstance()
{
    if(!instance)
        instance = new Configuration();
    return instance;
}

void Configuration::writeSystemInfo()
{
    QFile sysinfoFile(configFileDirPath()+"/sysinfo.txt");
//    if(sysinfoFile.exists())
//        return;
    if(!sysinfoFile.open(QIODevice::WriteOnly))
        return;
    QString osVersion;
#ifdef Q_OS_WIN
    osVersion = QString::fromLatin1("Windows %1").arg(QSysInfo::windowsVersion());
#elif Q_OS_LINUX
    osVersion = QString::fromLatin1("Linux");
#else
    osVersion = QString::fromLatin1("Mac OS X %1").arg(QSysInfo::macVersion());
#endif
    QString fileContent = QString::fromLatin1("MdCharm Version: %1\n"
                                              "MdCharm Revision: %2\n"
                                              "Program File Location: %3\n"
                                              "OS Version: %4").arg(VERSION_STR)
                                                .arg(REVISION_STR)
                                                .arg(qApp->applicationDirPath())
                                                .arg(osVersion);
    sysinfoFile.write(fileContent.toLocal8Bit());
    sysinfoFile.close();
}

QString Configuration::configFileDirPath()
{
#ifdef MDCHARM_DEBUG
    qDebug(settings->fileName().left(settings->fileName().lastIndexOf("/")).toLatin1());
#endif
    return settings->fileName().left(settings->fileName().lastIndexOf("/"));
}

void Configuration::setRecentFileList(const QStringList sl)
{
    settings->setValue(RECENT_FILE_LIST, sl);
}

QStringList Configuration::getRecentFileList()
{
    QVariant var = settings->value(RECENT_FILE_LIST);
    if(var.isValid() && var.canConvert(QVariant::StringList))
    {
        return var.toStringList();
    } else {
        setRecentFileList(QStringList());
        return QStringList();
    }
}

void Configuration::removeFromRecentFileList(const QString &toRemove)
{
    QStringList rl = getRecentFileList();
    rl.removeAll(toRemove);
    setRecentFileList(rl);
}

bool Configuration::isUseMarkdownDefaultCSS()
{
    QVariant var = settings->value(MARKDOWN_USE_DEFAULT_CSS);
    if(var.isValid() && var.canConvert(QVariant::String) && !var.toString().isEmpty())
        return false;//if string is not empty or null, it means user want to use him own css
    else
        return true;
}
// if b==true use custom css
void Configuration::setMarkdownCSS(bool isDefault, const QString customCSS)
{
    if(!isDefault)
    {
        QString markdownCSSFilePath = configFileDirPath()+"/markdown.css";
        settings->setValue(MARKDOWN_USE_DEFAULT_CSS, markdownCSSFilePath);
        Utils::saveFile(markdownCSSFilePath, customCSS.toUtf8());
    }
    else
    {
        settings->setValue(MARKDOWN_USE_DEFAULT_CSS, QString());
    }
}

QString Configuration::getMarkdownCSS()
{
    QString cssFilePath;
    if(isUseMarkdownDefaultCSS())
    {
        cssFilePath = QString::fromLatin1(":/markdown/default.css");
    } else {
        QVariant var = settings->value(MARKDOWN_USE_DEFAULT_CSS);
        if(var.isValid() && var.canConvert(QVariant::String))
        {
            cssFilePath = var.toString();
            if(!QFile::exists(cssFilePath))
                cssFilePath = QString::fromLatin1(":/markdown/default.css");
        }
        else
            cssFilePath = QString::fromLatin1(":/markdown/default.css");
    }
    if(cssFilePath.isEmpty())
        return QString();
    QFile markdownCSSFile(cssFilePath);
    if(!markdownCSSFile.open(QIODevice::ReadOnly))
        return QString();
    QTextStream textStream(&markdownCSSFile);
    QString result = textStream.readAll();
    markdownCSSFile.close();
    return result;
}

void Configuration::setProjectDockWidgetVisible(bool b)
{
    settings->setValue(IS_PROJECT_DOCK_WIDGET_VISIBLE, b);
}

bool Configuration::isProjectDockWidgetVisible()
{
    QVariant var = settings->value(IS_PROJECT_DOCK_WIDGET_VISIBLE);
    if(var.isValid() && var.canConvert(QVariant::Bool))
    {
        return var.toBool();
    }
    else
    {
        setProjectDockWidgetVisible(true);
        return true;
    }
}

void Configuration::setTocDockWidgetVisible(bool b)
{
    settings->setValue(IS_TOC_DOCK_WIDGET_VISIBLE, b);
}

bool Configuration::isTocDockWidgetVisible()
{
    QVariant var = settings->value(IS_TOC_DOCK_WIDGET_VISIBLE);
    if(var.isValid() && var.canConvert(QVariant::Bool))
        return var.toBool();
    else {
        setTocDockWidgetVisible(true);
        return true;
    }
}

void Configuration::setDefaultEncoding(const QString &defaultEncoding)
{
    settings->setValue(DEFAULT_ENCODING, defaultEncoding);
}

QString Configuration::getDefaultEncoding()
{
    QVariant var = settings->value(DEFAULT_ENCODING);
    if(var.isValid() && var.canConvert(QVariant::String))
    {
        return var.toString();
    } else {
        QString defaultEncoding = QString("UTF-8");
        setDefaultEncoding(defaultEncoding);
        return defaultEncoding;
    }
}

void Configuration::setUtf8BOMOptions(MdCharmGlobal::UTF8BOM ub)
{
    settings->setValue(UTF8_BOM, ub);
}

MdCharmGlobal::UTF8BOM Configuration::getUtf8BOMOptions()
{
    QVariant var = settings->value(UTF8_BOM);
    if(var.isValid() && var.canConvert(QVariant::Int))
    {
        switch(var.toInt())
        {
        case MdCharmGlobal::Add:
            return MdCharmGlobal::Add;
            break;
        case MdCharmGlobal::Delete:
            return MdCharmGlobal::Delete;
            break;
        default:
        case MdCharmGlobal::Keep:
            return MdCharmGlobal::Keep;
            break;
        }
    } else {
        setUtf8BOMOptions(MdCharmGlobal::Keep);
        return MdCharmGlobal::Keep;
    }
}

QString Configuration::getLanguageSpellCheckDictPath(const QString &lan)
{
    if(lan.isEmpty())
        return QString();
    //Check User Data Path First
    QString dicPath = QString("%1/%2/%3.dic").arg(configFileDirPath())
            .arg(SPELL_CHECK_DIC_DIRECTORY_NAME)
            .arg(lan);
    if(QFile::exists(dicPath))
        return dicPath;
    dicPath = QString("%1/%2/%3.dic").arg(defaultSpellCheckRootDir)
            .arg(SPELL_CHECK_DIC_DIRECTORY_NAME)
            .arg(lan);
    if(QFile::exists(dicPath))
        return dicPath;
    else
        return QString();
}

QString Configuration::getLanguageSpellCheckUserDictPath()
{
    return QString("%1/%2/user.txt").arg(configFileDirPath()).arg(SPELL_CHECK_DIC_DIRECTORY_NAME);
}

const QStringList Configuration::getAllAvailableSpellCheckDictNames()
{
    QStringList dicts;
    QString dirPath = QString("%1/%2").arg(defaultSpellCheckRootDir).arg(SPELL_CHECK_DIC_DIRECTORY_NAME);
    QDir dictDir(dirPath);
    if(!dictDir.exists())
        return dicts;
    QStringList filter;
    filter << "*.aff";
    QFileInfoList fileInfoList = dictDir.entryInfoList(filter, QDir::Files, QDir::Name);
    for(int i=0; i<fileInfoList.size(); i++){
        QFileInfo fileInfo = fileInfoList.at(i);
        QString dictFilePath = QString("%1/%2.dic").arg(fileInfo.absolutePath()).arg(fileInfo.baseName());
        if(QFile::exists(dictFilePath))
            dicts << fileInfo.baseName();
    }
    return dicts;
}

bool Configuration::isCheckSpell()
{
    QVariant var = settings->value(IS_ENABLE_SPELL_CHECK);
    if(var.isValid() && var.canConvert(QVariant::Bool))
    {
        return var.toBool();
    } else {
//        QLocale locale;
//        if(locale.name()=="en_US" || locale.name()=="en_GB")
//        {
//            setCheckSpell(true);
//            return true;
//        } else {
//            setCheckSpell(false);
//            return false;
//        }
        setCheckSpell(false);
        return false;
    }
}

void Configuration::setCheckSpell(bool b)
{
    settings->setValue(IS_ENABLE_SPELL_CHECK, b);
}

QString Configuration::getSpellCheckLanguage()
{
    QVariant var = settings->value(SPELL_CHECK_LANGUAGE);
    if(var.isValid() && var.canConvert(QVariant::String)){
        return var.toString();
    } else {
//        QLocale locale;
//        QString localeName = locale.name();
//        if(localeName=="en_US" || localeName=="en_GB"){
//            setSpellCheckLanguage(localeName);
//            return localeName;
//        } else {
//            return QString();
//        }
        return QString();
    }
}

void Configuration::setSpellCheckLanguage(const QString &lan)
{
    settings->setValue(SPELL_CHECK_LANGUAGE, lan);
}

bool Configuration::isUseWhiteSpaceInsteadOfTab()
{
    QVariant var = settings->value(USE_WHITE_SPACE_INSTEAD_OF_TAB);
    if(var.isValid() && var.canConvert(QVariant::Bool)){
        return var.toBool();
    } else {
        setUseWhiteSpaceInsteadOfTab(true);
        return true;
    }
}

void Configuration::setUseWhiteSpaceInsteadOfTab(bool b)
{
    settings->setValue(USE_WHITE_SPACE_INSTEAD_OF_TAB, b);
}

const QByteArray Configuration::getGeometryState()
{
    QVariant var = settings->value(GEOMETRY_STATE);
    if(var.isValid() && var.canConvert(QVariant::ByteArray)){
        return var.toByteArray();
    } else {
        return QByteArray();
    }
}

void Configuration::setGeometryState(const QByteArray &geometryState)
{
    settings->setValue(GEOMETRY_STATE, geometryState);
}

const QByteArray Configuration::getWindowState()
{
    QVariant var = settings->value(WINDOW_STATE);
    if(var.isValid() && var.canConvert(QVariant::ByteArray)){
        return var.toByteArray();
    } else {
        return QByteArray();
    }
}

void Configuration::setWindowState(const QByteArray &windowState)
{
    settings->setValue(WINDOW_STATE, windowState);
}

bool Configuration::isHaveValidApplicationState()
{
    return (!getGeometryState().isEmpty())&&(!getWindowState().isEmpty());
}

const QString Configuration::getSessionFilePath()
{
    return QString("%1/session.xml").arg(configFileDirPath());
}

bool Configuration::isAppendCodeSyntaxCss()
{
    QVariant var = settings->value(APPEND_CODE_SYNTAX_CSS);
    if(var.isValid() && var.canConvert(QVariant::Bool)){
        return var.toBool();
    } else {
        return false;
    }
}

void Configuration::setAppendCodeSyntaxCss(bool b)
{
    settings->setValue(APPEND_CODE_SYNTAX_CSS, b);
}

void Configuration::setLastOpenDir(const QString &dir)
{
    settings->setValue(LAST_OPEN_DIR, dir);
}

const QString Configuration::getLastOpenDir()
{
    QVariant var = settings->value(LAST_OPEN_DIR);
    if(var.isValid() && var.canConvert(QVariant::String))
        return var.toString();
    else
        return QDir::homePath();
}

bool Configuration::isHideFileExtensionInProjectDock()
{
    QVariant var = settings->value(HIDE_FILE_EXT_IN_PROJECT_DOCK);
    if(var.isValid() && var.canConvert(QVariant::String))
        return var.toBool();
    else
        return false;
}

void Configuration::setHideFileExtensionInProjectDock(bool b)
{
    settings->setValue(HIDE_FILE_EXT_IN_PROJECT_DOCK, b);
}

void Configuration::setFileExtension(const QStringList &extensions)
{
    settings->setValue(FILE_EXTENSION, extensions);
}

int Configuration::getLastFilterType()
{
    QVariant var = settings->value(LAST_FILTER_TYPE);
    if(var.isValid() && var.canConvert(QVariant::Int))
        return var.toInt();
    else
        return MarkdownFile;
}

void Configuration::setLastFilterType(int ft)
{
    settings->setValue(LAST_FILTER_TYPE, ft);
}

QStringList Configuration::getFileExtension()
{
    QVariant var = settings->value(FILE_EXTENSION);
    if(var.isValid() && var.canConvert(QVariant::StringList)){
        QStringList result = var.toStringList();
        if(result.length()<FileTypeNum){//Auto-add can't delete
            for(int i=result.length(); i<FileTypeNum; i++){
                result.append(fileTypeToString(i));
            }
            setFileExtension(result);
        }
        return result;
    } else {
        setFileExtension(fileTypeSuffixString);
        return fileTypeSuffixString;
    }
}

QString Configuration::fileTypeToString(int fileType)
{
    if(fileType>=fileTypeString.length()){
#ifdef MDCHARM_DEBUG
        Q_ASSERT(0 && "this should not be happen");
#endif
        return QString();
    }
    return fileTypeString.at(fileType);
}

QStringList Configuration::getFileOpenFilters(FileType firstType)
{
    QStringList exts = getFileExtension();
    QStringList filterList;
    for(int i=0; i<FileTypeNum && i<exts.length(); i++){
        QString filter = QString::fromLatin1("%1 (%2)").arg(fileTypeToString(i)).arg(exts.value(i).split("|").join(" "));
        if(firstType==i)
            filterList.prepend(filter);
        else
            filterList.append(filter);
    }
    return filterList;
}

QStringList Configuration::getFileFilter(FileType fy)
{
    QStringList exts = getFileExtension();
    return exts.value(fy).split("|");
}

void Configuration::setAutoIndentation(bool i)
{
    settings->setValue(AUTO_INDENTATION, i);
}

bool Configuration::isAutoIndentation()
{
    QVariant var = settings->value(AUTO_INDENTATION);
    if(var.isValid() && var.canConvert(QVariant::Bool)){
        return var.toBool();
    } else {
        setAutoIndentation(true);
        return true;
    }
}

void Configuration::setAutoPair(bool i)
{
    settings->setValue(AUTO_PAIR, i);
}

bool Configuration::isAutoPair()
{
    QVariant var = settings->value(AUTO_PAIR);
    if(var.isValid() && var.canConvert(QVariant::Bool)){
        return var.toBool();
    } else {
        setAutoPair(false);
        return false;
    }
}

void Configuration::setLastIgnoreRevision(const QString &rev)
{
    settings->setValue(LAST_IGNORE_REVISION, rev);
}

QString Configuration::getLastIgnoreRevision() const
{
    QVariant var = settings->value(LAST_IGNORE_REVISION);
    if(var.isValid() && var.canConvert(QVariant::String)){
        return var.toString();
    } else {
        return QString();
    }
}

bool Configuration::isDisplayRightColumnMargin() const
{
    return getRightMarginColumn() > 0;
}

int Configuration::getRightMarginColumn() const
{
    QVariant var = settings->value(RIGHT_MARGIN_COLUMN);
    if(var.isValid() && var.canConvert(QVariant::Int)){
        return var.toInt();
    } else {
        return -1;
    }
}

void Configuration::setRightMarginColumn(int column)
{
    settings->setValue(RIGHT_MARGIN_COLUMN, column);
}

void Configuration::setMarkdownEngineType(MarkdownToHtml::MarkdownType type)
{
    settings->setValue(MARKDOWN_ENGINE, type);
}

MarkdownToHtml::MarkdownType Configuration::getMarkdownEngineType() const
{
    QVariant var = settings->value(MARKDOWN_ENGINE);
    if(var.isValid() && var.canConvert(QVariant::Int)){
        int type = var.toInt();
        switch(type)
        {
            case MarkdownToHtml::MultiMarkdown:
                return MarkdownToHtml::MultiMarkdown;
                break;
            default:
                return MarkdownToHtml::PHPMarkdownExtra;
                break;
        }
    } else {
        return MarkdownToHtml::PHPMarkdownExtra;
    }
}

QVariant Configuration::getLastStateValue(const QString &key) const
{
    return settings->value(LAST_STATE_GROUP+key);
}

void Configuration::setLastStateValue(const QString &key, const QVariant &value)
{
    settings->setValue(LAST_STATE_GROUP+key, value);
}

QString Configuration::getKeyboardShortcut(int s)
{
    QVariant var = settings->value(SHORTCUTS_GROUP+getShortcutKeyName(s));
    if(var.isValid() && var.canConvert(QVariant::String)){
        return var.toString();
    } else {
        QString k = getKeyboardDefaultShortcut(s);
        setKeyboardShortcut(s, k);
        return k;
    }
}

void Configuration::setKeyboardShortcut(int s, const QString &keys)
{
    settings->setValue(SHORTCUTS_GROUP+getShortcutKeyName(s), keys);
}

QString Configuration::getShortcutKeyName(int s)
{
    switch (s) {
        case MdCharmGlobal::ShortcutCloseTab:
            return "Close_Tab";
            break;
        case MdCharmGlobal::ShortcutItalic:
            return "Italic_Text";
            break;
        case MdCharmGlobal::ShortcutBold:
            return "Bold_Text";
            break;
        case MdCharmGlobal::ShortcutQuoteText:
            return "Quote_Text";
            break;
        case MdCharmGlobal::ShortcutShiftTab:
            return "Untab_Block_Text";
            break;
        case MdCharmGlobal::ShortcutStrikeThrough:
            return "Strike_Through_Text";
            break;
        case MdCharmGlobal::ShortcutInsertLink:
            return "Insert_Link";
            break;
        case MdCharmGlobal::ShortcutInsertPicture:
            return "Insert_Picture";
            break;
        case MdCharmGlobal::ShortcutCheatSheet:
            return "Open_Cheat_Sheet";
            break;
        case MdCharmGlobal::ShortcutInsertCode:
            return "Insert_Code";
            break;
        case MdCharmGlobal::ShortcutNew:
            return "New";
            break;
        case MdCharmGlobal::ShortcutOpen:
            return "Open";
            break;
        case MdCharmGlobal::ShortcutSave:
            return "Save";
            break;
        case MdCharmGlobal::ShortcutSaveAs:
            return "SaveAs";
            break;
        case MdCharmGlobal::ShortcutPrint:
            return "Print";
            break;
        case MdCharmGlobal::ShortcutQuit:
            return "Quit";
            break;
        case MdCharmGlobal::ShortcutUndo:
            return "Undo";
            break;
        case MdCharmGlobal::ShortcutRedo:
            return "Redo";
            break;
        case MdCharmGlobal::ShortcutCut:
            return "Cut";
            break;
        case MdCharmGlobal::ShortcutCopy:
            return "Copy";
            break;
        case MdCharmGlobal::ShortcutPaste:
            return "Paste";
            break;
        case MdCharmGlobal::ShortcutSelectAll:
            return "Select_All";
            break;
        case MdCharmGlobal::ShortcutFind:
            return "Find";
            break;
        case MdCharmGlobal::ShortcutFindPrevious:
            return "Find_Previous";
            break;
        case MdCharmGlobal::ShortcutFindNext:
            return "Find_Next";
            break;
        case MdCharmGlobal::ShortcutHelpContents:
            return "Help";
            break;
        case MdCharmGlobal::ShortcutPrintPreview:
            return "Print_Preview";
            break;
        case MdCharmGlobal::ShortcutHideProjectDockBar:
            return "Hide_Project_Dock_Bar";
            break;
        default:
            Q_ASSERT(0 && "This should not be happend");
            return "";
            break;
    }
}

QString Configuration::getKeyboardDefaultShortcut(int s)
{
    switch (s) {
        case MdCharmGlobal::ShortcutCloseTab:
            return "Ctrl+W";
            break;
        case MdCharmGlobal::ShortcutItalic:
            return QKeySequence(QKeySequence::Italic).toString();
            break;
        case MdCharmGlobal::ShortcutBold:
            return QKeySequence(QKeySequence::Bold).toString();
            break;
        case MdCharmGlobal::ShortcutQuoteText:
            return "Ctrl+Q";
            break;
        case MdCharmGlobal::ShortcutShiftTab:
            return "Shift+Tab";
            break;
        case MdCharmGlobal::ShortcutStrikeThrough:
            return "Ctrl+T";
            break;
        case MdCharmGlobal::ShortcutInsertLink:
            return "Ctrl+L";
            break;
        case MdCharmGlobal::ShortcutInsertPicture:
            return "Ctrl+Shift+L";
            break;
        case MdCharmGlobal::ShortcutCheatSheet:
            return "Ctrl+M";
            break;
        case MdCharmGlobal::ShortcutInsertCode:
            return "Ctrl+K";
            break;
        case MdCharmGlobal::ShortcutNew:
            return QKeySequence(QKeySequence::New).toString();
            break;
        case MdCharmGlobal::ShortcutOpen:
            return QKeySequence(QKeySequence::Open).toString();
            break;
        case MdCharmGlobal::ShortcutSave:
            return QKeySequence(QKeySequence::Save).toString();
            break;
        case MdCharmGlobal::ShortcutSaveAs:
            return QKeySequence(QKeySequence::SaveAs).toString();
            break;
        case MdCharmGlobal::ShortcutPrint:
            return QKeySequence(QKeySequence::Print).toString();
            break;
        case MdCharmGlobal::ShortcutQuit:
            return "Alt+F4";
            break;
        case MdCharmGlobal::ShortcutUndo:
            return QKeySequence(QKeySequence::Undo).toString();
            break;
        case MdCharmGlobal::ShortcutRedo:
            return QKeySequence(QKeySequence::Redo).toString();
            break;
        case MdCharmGlobal::ShortcutCut:
            return QKeySequence(QKeySequence::Cut).toString();
            break;
        case MdCharmGlobal::ShortcutCopy:
            return QKeySequence(QKeySequence::Copy).toString();
            break;
        case MdCharmGlobal::ShortcutPaste:
            return QKeySequence(QKeySequence::Paste).toString();
            break;
        case MdCharmGlobal::ShortcutSelectAll:
            return QKeySequence(QKeySequence::SelectAll).toString();
            break;
        case MdCharmGlobal::ShortcutFind:
            return QKeySequence(QKeySequence::Find).toString();
            break;
        case MdCharmGlobal::ShortcutFindPrevious:
            return QKeySequence(QKeySequence::FindPrevious).toString();
            break;
        case MdCharmGlobal::ShortcutFindNext:
            return QKeySequence(QKeySequence::FindNext).toString();
            break;
        case MdCharmGlobal::ShortcutHelpContents:
            return QKeySequence(QKeySequence::HelpContents).toString();
            break;
        case MdCharmGlobal::ShortcutPrintPreview:
            return QKeySequence(Qt::CTRL+Qt::SHIFT+Qt::Key_P).toString();
            break;
        case MdCharmGlobal::ShortcutHideProjectDockBar:
            return QKeySequence(Qt::ALT+Qt::Key_F2).toString();
            break;
        default:
            Q_ASSERT(0 && "This should not be happen");
            return QString("");
            break;
    }
}
