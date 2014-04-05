#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <QString>
#include "utils.h"
#include "markdowntohtml.h"

class QSettings;

class Configuration : public QObject
{
    Q_OBJECT
public:
    /*!
     * \brief The FileType enum
     * \warning If we support new file type
     *          1) must add file type in this
     *          2) must append file type desc string to fileTypeString
     *          3) must append file type suffix string to fileTypeSuffixString
     */
    enum FileType
    {
        AllFile=0,
        MarkdownFile=1,
        FileTypeNum
    };
public:
    static Configuration *getInstance();

    void setFontFamily(const QString &family);
    QString getFontFamily();
    void setFontSize(const int fontSize);
    int getFontSize();
    void setTabSize(const int tabSize);
    int getTabSize();
    void setHighlightCurrentLine(const bool b);
    bool isHighlightCurrentLine();
    void setEnableTextWrapping(const bool b);
    bool isEnableTextWrapping();
    void setDisplayLineNumber(const bool b);
    bool isDisplayLineNumber();
    void setPreviewOption(const int type);
    bool isShowPreview_Old();
    int getPreviewOption();
    void changeSyncScrollbarSetting(bool sync);
    bool isSyncScrollbar();
    void setShowSplash(const bool b);
    bool isShowSplash();
    void setCheckForUpdates(const bool b);
    bool isCheckForUpdates();
    void setRecentFileList(const QStringList sl);
    QStringList getRecentFileList();
    void removeFromRecentFileList(const QString &toRemove);
    bool isUseMarkdownDefaultCSS();
    void setMarkdownCSS(bool isDefault, const QString customCSS=QString());
    QString getMarkdownCSS();
    void setProjectDockWidgetVisible(bool b);
    bool isProjectDockWidgetVisible();
    void setDefaultEncoding(const QString &defaultEncoding);
    QString getDefaultEncoding();
    void setUtf8BOMOptions(MdCharmGlobal::UTF8BOM ub);
    MdCharmGlobal::UTF8BOM getUtf8BOMOptions();
    QString getLanguageSpellCheckDictPath(const QString &lan);
    QString getLanguageSpellCheckUserDictPath();
    const QStringList getAllAvailableSpellCheckDictNames();
    bool isCheckSpell();
    void setCheckSpell(bool b);
    QString getSpellCheckLanguage();
    void setSpellCheckLanguage(const QString &lan);
    bool isUseWhiteSpaceInsteadOfTab();
    void setUseWhiteSpaceInsteadOfTab(bool b);
    const QByteArray getGeometryState();
    void setGeometryState(const QByteArray &geometryState);
    const QByteArray getWindowState();
    void setWindowState(const QByteArray &windowState);
    bool isHaveValidApplicationState();
    const QString getSessionFilePath();
    bool isAppendCodeSyntaxCss();
    void setAppendCodeSyntaxCss(bool b);
    void setLastOpenDir(const QString& dir);
    const QString getLastOpenDir();
    void setFileExtension(const QStringList& extensions);
    int getLastFilterType();
    void setLastFilterType(int ft);
    QStringList getFileExtension();
    QString fileTypeToString(int order);
    QStringList getFileOpenFilters(FileType firstType=AllFile);
    QStringList getFileFilter(FileType fy);
    void setAutoIndentation(bool i);
    bool isAutoIndentation();
    void setAutoPair(bool i);
    bool isAutoPair();
    void setLastIgnoreRevision(const QString &rev);
    QString getLastIgnoreRevision() const;
    bool isDisplayRightColumnMargin() const;
    int getRightMarginColumn() const;
    void setRightMarginColumn(int column);
    void setMarkdownEngineType(MarkdownToHtml::MarkdownType type);
    MarkdownToHtml::MarkdownType getMarkdownEngineType() const;
    QVariant getLastStateValue(const QString &key) const;
    void setLastStateValue(const QString &key, const QVariant &value);
    QString getKeyboardShortcut(int s);
    void setKeyboardShortcut(int s, const QString &keys);
    QString getKeyboardDefaultShortcut(int s);
    QString getShortcutKeyName(int s);

    QString getLicense() const;
    void setLicense(const QString &license);
    QDateTime getLastCheckDatetime() const;
    void setLastCheckDatetime(const QDateTime &datetime);
    QDateTime getFirstUseDatetime() const;

    QString configFileDirPath();
    void writeSystemInfo();

    ~Configuration();
private:
    Configuration();
private:
    static Configuration *instance;
    QString defaultSpellCheckRootDir;
    QSettings *settings;
    QSettings *regSettings;

    static const QString FONT_FAMILY;
    static const QString FONT_SIZE;
    static const QString TAB_SIZE;
    static const QString DISPLAY_LINE_NUMBER;
    static const QString HIGHLIGHT_CURRENT_LINE;
    static const QString ENABLE_TEXT_WRAPPING;
    static const QString PREVIEW_OPTION;
    static const QString IS_SHOW_SPLASH;
    static const QString IS_CHECK_FOR_UPDATES;
    static const QString RECENT_FILE_LIST;
    static const QString MARKDOWN_USE_DEFAULT_CSS;
    static const QString IS_PROJECT_DOCK_WIDGET_VISIBLE;
    static const QString DEFAULT_ENCODING;
    static const QString UTF8_BOM;
    static const QString SYNC_SCROLLBAR;
    static const QString IS_ENABLE_SPELL_CHECK;
    static const QString SPELL_CHECK_LANGUAGE;
    static const QString USE_WHITE_SPACE_INSTEAD_OF_TAB;
    static const QString GEOMETRY_STATE;
    static const QString WINDOW_STATE;
    static const QString APPEND_CODE_SYNTAX_CSS;
    static const QString LAST_OPEN_DIR;
    static const QString FILE_EXTENSION;
    static const QString LAST_FILTER_TYPE;
    static const QString AUTO_INDENTATION;
    static const QString AUTO_PAIR;
    static const QString LAST_IGNORE_REVISION;
    static const QString RIGHT_MARGIN_COLUMN;
    static const QString MARKDOWN_ENGINE;
    static const QString LAST_STATE_GROUP;
    static const QString SHORTCUTS_GROUP;

    static const QString NATIVE_LICENSE;
    static const QString NATIVE_LAST_CHECK_DATETIME;
    static const QString NATIVE_FIRST_USE_DATETIME;

    QStringList fileTypeString;
    QStringList fileTypeSuffixString;
};

#endif // CONFIGURATION_H
