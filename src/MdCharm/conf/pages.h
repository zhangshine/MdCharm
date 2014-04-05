#ifndef PAGES_H
#define PAGES_H

#include <QWidget>
#include <QTabWidget>
#include <QTableView>
#include <QStandardItemModel>

#include "../configuration.h"
#include "util/syntax/hightlighter.h"

class QFontComboBox;
class QComboBox;
class QSpinBox;
class QCheckBox;
class QPlainTextEdit;
class QLabel;
class ShortcutLineEdit;
class QPushButton;

/*---------------------------------Example--------------------------------------

class Pages : public QWidget
{
    Q_OBJECT
public:
    explicit Pages(QWidget *parent = 0);
    
signals:
    
public slots:
    
};

------------------------------------------------------------------------------*/
namespace Ui {
class EnvPage;
class TextEditorPage;
class StylesPage;
}
class EnvironmentPage : public QTabWidget
{
    Q_OBJECT
public:
    explicit EnvironmentPage(QWidget *parent = 0);
    ~EnvironmentPage();
    void saveConfig();
private:
    void reCalculateExts();
    bool isFileExtAlreadyExists(QString &text);
    void initKeyboardData();
    void recalculateKeyMapDuplicate();
protected:
    void resizeEvent(QResizeEvent *e);
signals:
    void updateShortcut(int s, const QString &text);
private slots:
    void fileTypeListWidgetCurrentRowChangedSlot(int currentRow);
    void deleteToolBtnSlot();
    void addToolBtnSlot();
    void onShortcutLineEditChanged(const QString &text);
    void onShortcutItemClicked(const QModelIndex &index);
    void resetKeyboardShortcut();
private:
    Ui::EnvPage *ui;
    Configuration *conf;
    QCheckBox *showSplashCheckBox;
    QCheckBox *checkUpdatesCheckBox;
    QStandardItemModel *keyboardModel;

    //Shortcut
    QLabel *shortcutLabel;
    ShortcutLineEdit *shortcutLineEdit;
    QPushButton *resetShortcutPushButton;
};

class TextEditorPage : public QTabWidget
{
    Q_OBJECT
public:
    explicit TextEditorPage(QWidget *parent = 0);
    ~TextEditorPage();
    void saveConfig();

private:
    int getFontSizeIndex(const int size);
    void prepareDefaultEncodingData();
    void prepareUtf8BOMData();
    void prepareSpellCheckLanguageData();
    void prepareTabOptionData();
signals:

public slots:
private slots:
    void spellCheckCheckBoxSlot(bool isChecked);
    void useWhiteSpaceInsteadOfTabSlot(bool isChecked);
private:
    Ui::TextEditorPage *ui;
    Configuration *conf;
    MdCharmGlobal *mdcharmGlobal;
    QFontComboBox *fontFamilyComboBox;
    QComboBox *fontSizeComboBox;
    QComboBox *defaultEncodingComboBox;
    QComboBox *utf8BOMComboBox;
    QSpinBox *tabSizeSpinBox;
    QCheckBox *enableTWCheckBox;
    QCheckBox *displayLineNumberCheckBox;
    QCheckBox *highlightCLCheckBox;
};

class StylesPage : public QTabWidget
{
    Q_OBJECT
public:
    explicit StylesPage(QWidget *parent = 0);
    ~StylesPage();
    void saveConfig();
private:
    Ui::StylesPage *ui;
    Configuration *conf;
    QCheckBox *useDefaultCheckBox;
    QPlainTextEdit *customCSSPlainTextEdit;
    CSSHighLighter *cssHighLighter;
private slots:
    void checkStateChanged();
};

#endif // PAGES_H
