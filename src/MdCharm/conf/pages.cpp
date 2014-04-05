#include <QtGui>
#include <cassert>

#include "markdowntohtml.h"
#include "pages.h"
#include "../utils.h"
#include "ui_environmentpage.h"
#include "ui_texteditorpage.h"
#include "ui_stylespage.h"
#include "util/gui/shortcutlineedit.h"
//---------------------- EnvironmentPage ---------------------------------------
EnvironmentPage::EnvironmentPage(QWidget *parent) :
    QTabWidget(parent),
    ui(new Ui::EnvPage)

{
    ui->setupUi(this);
    conf = Configuration::getInstance();
    showSplashCheckBox = ui->showSplashCheckBox;
    showSplashCheckBox->setChecked(conf->isShowSplash());
    checkUpdatesCheckBox = ui->checkUpdatesCheckBox;
    checkUpdatesCheckBox->setChecked(conf->isCheckForUpdates());
    for(int i=1; i<Configuration::FileTypeNum; i++){
        ui->fileTypeListWidget->addItem(conf->fileTypeToString(i));
    }
    if(Configuration::FileTypeNum>=2){
        ui->fileTypeListWidget->setCurrentRow(0);
        QString exts = conf->getFileExtension().at(1);
        QStringList extsList = exts.split("|");
        ui->extensionListWidget->addItems(extsList);
    }
    ui->markdownEngineComboBox->addItem(tr("Default"), MarkdownToHtml::PHPMarkdownExtra);
    ui->markdownEngineComboBox->addItem(tr("MultiMarkdown"), MarkdownToHtml::MultiMarkdown);
    ui->markdownEngineComboBox->setCurrentIndex(conf->getMarkdownEngineType()==MarkdownToHtml::PHPMarkdownExtra ? 0 : 1);

    initKeyboardData();

    connect(ui->fileTypeListWidget, SIGNAL(currentRowChanged(int)),
            this, SLOT(fileTypeListWidgetCurrentRowChangedSlot(int)));
    connect(ui->addToolButton, SIGNAL(clicked()),
            this, SLOT(addToolBtnSlot()));
    connect(ui->deleteToolButton, SIGNAL(clicked()),
            this, SLOT(deleteToolBtnSlot()));
}

EnvironmentPage::~EnvironmentPage()
{
    delete ui;
}

void EnvironmentPage::saveConfig()
{
    conf->setShowSplash(showSplashCheckBox->isChecked());
    conf->setCheckForUpdates(checkUpdatesCheckBox->isChecked());
    int index = ui->markdownEngineComboBox->currentIndex();
    int type = ui->markdownEngineComboBox->itemData(index).toInt();
    switch (type) {
        case MarkdownToHtml::MultiMarkdown:
            conf->setMarkdownEngineType(MarkdownToHtml::MultiMarkdown);
            break;
        default:
            conf->setMarkdownEngineType(MarkdownToHtml::PHPMarkdownExtra);
            break;
    }

}

void EnvironmentPage::fileTypeListWidgetCurrentRowChangedSlot(int currentRow)
{
    ui->extensionListWidget->clear();
    ui->extensionListWidget->addItems(conf->getFileExtension().at(currentRow+1).split("|"));
}

void EnvironmentPage::addToolBtnSlot()
{
    QString target = ui->newExtensionLineEdit->text().trimmed();
    if(target.isEmpty())
        return;
    if(isFileExtAlreadyExists(target)){
        //warning and return
        QMessageBox::warning(this, tr("File extension already in use"), tr("File extension already in use"));
        return;
    }
    ui->extensionListWidget->addItem(target);
    reCalculateExts();
    ui->newExtensionLineEdit->clear();
}

void EnvironmentPage::onShortcutLineEditChanged(const QString &text)
{
    QModelIndex index = ui->shortcutTableView->currentIndex();
    if(!index.isValid())
        return;
    QModelIndex keyIndex = keyboardModel->index(index.row(), 1);
    if(!keyIndex.isValid())
        return;
    if(keyIndex.data().toString()==text)
        return;
    keyboardModel->setData(keyIndex, text);
    recalculateKeyMapDuplicate();
    QStandardItem* item = keyboardModel->itemFromIndex(keyIndex);
    if(!item)
        return;
    conf->setKeyboardShortcut(item->data().toInt(), text);
    emit updateShortcut( item->data().toInt(), text);
}

void EnvironmentPage::recalculateKeyMapDuplicate()
{
    QMap<QString, int> checkKeyMap;
    for(int i=0; i<keyboardModel->rowCount(); i++){
        QModelIndex ix = keyboardModel->index(i, 1);
        if(!ix.isValid())
            return;
        if(checkKeyMap.contains(ix.data().toString()))
            checkKeyMap[ix.data().toString()]++;
        else
            checkKeyMap[ix.data().toString()] = 1;
        keyboardModel->setData(ix, QColor(Qt::black), Qt::ForegroundRole);
    }
    QMapIterator<QString, int> it(checkKeyMap);
    while(it.hasNext()){
        it.next();
        if(it.value()>1){
            for(int i=0; i<keyboardModel->rowCount(); i++){
                QModelIndex ix = keyboardModel->index(i, 1);
                if(!ix.isValid())
                    return;
                if(ix.data().toString()==it.key())
                    keyboardModel->setData(ix, QColor(Qt::red), Qt::ForegroundRole);
            }
        }
    }
}

void EnvironmentPage::onShortcutItemClicked(const QModelIndex &index)
{
    QModelIndex keyIndex = keyboardModel->index(index.row(), 1);
    if(!keyIndex.isValid())
        return;
    shortcutLineEdit->setText(keyIndex.data().toString());
    shortcutLineEdit->setEnabled(true);
    resetShortcutPushButton->setEnabled(true);
}

void EnvironmentPage::resetKeyboardShortcut()
{
    QModelIndex index = ui->shortcutTableView->currentIndex();
    if(!index.isValid())
        return;
    QStandardItem* item = keyboardModel->itemFromIndex(index);
    if(!item)
        return;
    QString key = conf->getKeyboardDefaultShortcut(item->data().toInt());
    shortcutLineEdit->setText(key);
    onShortcutLineEditChanged(key);
}

void EnvironmentPage::deleteToolBtnSlot()
{
    QListWidgetItem *r = ui->extensionListWidget->takeItem(ui->extensionListWidget->currentRow());
    delete r;
    reCalculateExts();
}

void EnvironmentPage::reCalculateExts()
{
    QStringList exts = conf->getFileExtension();
    int type = ui->fileTypeListWidget->currentRow() + 1;
    if(type>=exts.length()){
#ifdef MDCHARM_DEBUG
        Q_ASSERT(0 && "this should not be happend");
#endif
        return;
    }

    QStringList newExts;
    for(int i=0; i<ui->extensionListWidget->count(); i++){
        QListWidgetItem* cur = ui->extensionListWidget->item(i);
        newExts << cur->text();
    }
    exts[type] = newExts.join("|");
    conf->setFileExtension(exts);
}

bool EnvironmentPage::isFileExtAlreadyExists(QString &text)
{
    for(int i=0; i<ui->extensionListWidget->count(); i++){
        if(ui->extensionListWidget->item(i)->text()==text)
            return true;
    }
    return false;
}

void EnvironmentPage::initKeyboardData()
{
    shortcutLabel = new QLabel(tr("Shortcut:"), this);
    shortcutLineEdit = new ShortcutLineEdit(this);
    shortcutLineEdit->setEnabled(false);
    resetShortcutPushButton = new QPushButton(tr("reset"), this);
    resetShortcutPushButton->setEnabled(false);
    QHBoxLayout *shortcutLayout = new QHBoxLayout;
    shortcutLayout->addWidget(shortcutLabel);
    shortcutLayout->addWidget(shortcutLineEdit);
    shortcutLayout->addWidget(resetShortcutPushButton);
    ui->keyboardVerticalLayout->addLayout(shortcutLayout);

    keyboardModel = new QStandardItemModel(this);
    keyboardModel->setColumnCount(2);
    keyboardModel->setHeaderData(0, Qt::Horizontal, tr("Command"));
    keyboardModel->setHeaderData(1, Qt::Horizontal, tr("Shortcut"));

    QList<QStandardItem *> items;
    QStandardItem *ic, *is;
    MdCharmGlobal *global = MdCharmGlobal::getInstance();
    for(int i=0; i<MdCharmGlobal::ShortcutEnd; i++){
        items.clear();
        ic = new QStandardItem(global->getShortDescriptionText(i));
        ic->setData(i);
        is = new QStandardItem(conf->getKeyboardShortcut(i));
        is->setData(i);
        items << ic << is;
        keyboardModel->appendRow(items);
    }
    recalculateKeyMapDuplicate();
    ui->shortcutTableView->setModel(keyboardModel);


    connect(ui->shortcutTableView, SIGNAL(clicked(QModelIndex)), this, SLOT(onShortcutItemClicked(QModelIndex)));
    connect(shortcutLineEdit, SIGNAL(textEdited(QString)), this, SLOT(onShortcutLineEditChanged(QString)));
    connect(resetShortcutPushButton, SIGNAL(clicked()), this, SLOT(resetKeyboardShortcut()));
}

void EnvironmentPage::resizeEvent(QResizeEvent *e)
{
    ui->shortcutTableView->setColumnWidth(0, e->size().width()*0.64);
    ui->shortcutTableView->setColumnWidth(1, e->size().width()*0.2);
    QTabWidget::resizeEvent(e);
}

//------------------------- TextEditorPage -------------------------------------
TextEditorPage::TextEditorPage(QWidget *parent) :
    QTabWidget(parent),
    ui(new Ui::TextEditorPage)
{
    ui->setupUi(this);
    conf = Configuration::getInstance();
    mdcharmGlobal = MdCharmGlobal::getInstance();
    fontFamilyComboBox = ui->fontComboBox;
    fontSizeComboBox = ui->fontSizeComboBox;
    tabSizeSpinBox = ui->tabSizeSpinBox;
    enableTWCheckBox = ui->enableTWCheckBox;
    displayLineNumberCheckBox = ui->displayLineNumberCheckBox;
    highlightCLCheckBox = ui->highlightCLCheckBox;
    defaultEncodingComboBox = ui->defaultEncodingComboBox;
    utf8BOMComboBox = ui->utf8BOMComboBox;

    assert((fontSizeComboBox->count() > getFontSizeIndex(conf->getFontSize())));
    fontSizeComboBox->setCurrentIndex(getFontSizeIndex(conf->getFontSize()));
    fontFamilyComboBox->setCurrentFont(QFont(conf->getFontFamily()));
//    tabSizeSpinBox->setValue(conf->getTabSize());
    displayLineNumberCheckBox->setChecked(conf->isDisplayLineNumber());
    enableTWCheckBox->setChecked(conf->isEnableTextWrapping());
    highlightCLCheckBox->setChecked(conf->isHighlightCurrentLine());
    prepareTabOptionData();
    prepareDefaultEncodingData();
    prepareUtf8BOMData();
    prepareSpellCheckLanguageData();
    ui->autoIndentationCheckBox->setChecked(conf->isAutoIndentation());

    if(conf->isDisplayRightColumnMargin()){
        ui->rightMarginCheckBox->setChecked(true);
        ui->rightMarginSpinBox->setValue(conf->getRightMarginColumn());
    } else {
        ui->rightMarginCheckBox->setChecked(false);
        ui->rightMarginSpinBox->setEnabled(false);
        ui->rightMarginSpinBox->setValue(80);
    }
    ui->autoPairCheckBox->setChecked(conf->isAutoPair());
    connect(ui->rightMarginCheckBox, SIGNAL(toggled(bool)), ui->rightMarginSpinBox, SLOT(setEnabled(bool)));
}

TextEditorPage::~TextEditorPage()
{
    delete ui;
}

void TextEditorPage::saveConfig()
{
    // Font Family
    conf->setFontFamily(fontFamilyComboBox->currentFont().family());
    // Font Size
    bool b;
    int r = fontSizeComboBox->currentText().toInt(&b);
    conf->setFontSize(b ? r : 10);
    //Tab Key Width Size
    conf->setTabSize(tabSizeSpinBox->value());
    conf->setUseWhiteSpaceInsteadOfTab(ui->useSpaceInsteadCheckBox->isChecked());
    //Display Line Number
    conf->setDisplayLineNumber(displayLineNumberCheckBox->isChecked());
    //Highlight Current Line
    conf->setHighlightCurrentLine(highlightCLCheckBox->isChecked());
    //Enable Text Wrapping
    conf->setEnableTextWrapping(enableTWCheckBox->isChecked());
    //Default Encoding
    conf->setDefaultEncoding(defaultEncodingComboBox->currentText());
    //UTF-8 BOM Options
    MdCharmGlobal::UTF8BOM ub;
    switch(utf8BOMComboBox->itemData(utf8BOMComboBox->currentIndex()).toInt())
    {
    default:
    case MdCharmGlobal::Keep:
        ub = MdCharmGlobal::Keep;
        break;
    case MdCharmGlobal::Add:
        ub = MdCharmGlobal::Add;
        break;
    case MdCharmGlobal::Delete:
        ub = MdCharmGlobal::Delete;
        break;
    }
    conf->setUtf8BOMOptions(ub);
    conf->setCheckSpell(ui->spellCheckCheckBox->isChecked());
    if(conf->isCheckSpell())
        conf->setSpellCheckLanguage(ui->spellCheckComboBox->itemData(ui->spellCheckComboBox->currentIndex()).toString());
    //Auto Indentation
    conf->setAutoIndentation(ui->autoIndentationCheckBox->isChecked());
    //Right margin
    if(ui->rightMarginCheckBox->isChecked()){
        conf->setRightMarginColumn(ui->rightMarginSpinBox->value());
    } else {
        conf->setRightMarginColumn(-1);
    }

    if(ui->autoPairCheckBox->isChecked()!=conf->isAutoPair()){
        conf->setAutoPair(ui->autoPairCheckBox->isChecked());
    }
}

int TextEditorPage::getFontSizeIndex(const int size)
{
    QList<int> fontSizeList;
    fontSizeList << 6 << 7 << 8 << 9 << 10 << 11 << 12 << 14 << 16 << 18 << 20 << 22 << 24 << 26 << 28 << 36 << 48 << 72;
    for( int i=0; i < fontSizeList.size(); i++)
    {
        if (fontSizeList.at(i) == size)
            return i;
    }
    return 4;//default size 10
}

void TextEditorPage::prepareTabOptionData()
{
    if(conf->isUseWhiteSpaceInsteadOfTab()){
        ui->tabSizeSpinBox->setValue(4);
        ui->tabSizeSpinBox->setEnabled(false);
        ui->useSpaceInsteadCheckBox->setChecked(true);
    } else {
        ui->tabSizeSpinBox->setValue(conf->getTabSize());
    }
    connect(ui->useSpaceInsteadCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(useWhiteSpaceInsteadOfTabSlot(bool)));
}

void TextEditorPage::prepareDefaultEncodingData()
{
    QStringList sl = Utils::getEncodingList();
    defaultEncodingComboBox->addItems(sl);
    defaultEncodingComboBox->setCurrentIndex(sl.indexOf(conf->getDefaultEncoding()));
}

void TextEditorPage::prepareUtf8BOMData()
{
    utf8BOMComboBox->addItem(tr("Add If Encoding is UTF-8"), MdCharmGlobal::Add);
    utf8BOMComboBox->addItem(tr("Keep If Already Present"), MdCharmGlobal::Keep);
    utf8BOMComboBox->addItem(tr("Always Delete"), MdCharmGlobal::Delete);
    utf8BOMComboBox->setCurrentIndex(conf->getUtf8BOMOptions());
}

void TextEditorPage::prepareSpellCheckLanguageData()
{
    ui->spellCheckCheckBox->setChecked(conf->isCheckSpell());
    QStringList lanList;
    const QStringList allDicts = conf->getAllAvailableSpellCheckDictNames();
    for(int i=0; i<allDicts.length(); i++){
        QString dictName = allDicts.at(i);
        QString dictLocaleName = mdcharmGlobal->getDictLocaleName(dictName);
        if(dictLocaleName.isEmpty())
            dictLocaleName = dictName;
        ui->spellCheckComboBox->addItem(dictLocaleName, dictName);
        lanList << dictName;
    }
    if(!conf->getSpellCheckLanguage().isEmpty())
        ui->spellCheckComboBox->setCurrentIndex(lanList.indexOf(conf->getSpellCheckLanguage()));
    if(ui->spellCheckComboBox->currentIndex()==-1&&ui->spellCheckComboBox->count()>=1)
        ui->spellCheckComboBox->setCurrentIndex(0);
    ui->spellCheckComboBox->setEnabled(conf->isCheckSpell());
    connect(ui->spellCheckCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(spellCheckCheckBoxSlot(bool)));
}

void TextEditorPage::spellCheckCheckBoxSlot(bool isChecked)
{
    ui->spellCheckComboBox->setEnabled(isChecked);
}

void TextEditorPage::useWhiteSpaceInsteadOfTabSlot(bool isChecked)
{
    if(isChecked)
        ui->tabSizeSpinBox->setValue(4);
    ui->tabSizeSpinBox->setEnabled(!isChecked);
}
//----------------------------- StylesPage -------------------------------------
StylesPage::StylesPage(QWidget *parent) :
    QTabWidget(parent),
    ui(new Ui::StylesPage)
{
    ui->setupUi(this);
    conf = Configuration::getInstance();
    useDefaultCheckBox = ui->useDefaultCheckBox;
    customCSSPlainTextEdit = ui->customCSSPlainTextEdit;
    customCSSPlainTextEdit->setFont(QFont(conf->getFontFamily(),conf->getFontSize()));
    customCSSPlainTextEdit->setPlainText(conf->getMarkdownCSS());
    cssHighLighter = new CSSHighLighter(customCSSPlainTextEdit->document());
    if(conf->isUseMarkdownDefaultCSS())
    {
        useDefaultCheckBox->setChecked(true);
        customCSSPlainTextEdit->setReadOnly(true);
    }
    else
    {
        useDefaultCheckBox->setChecked(false);
        customCSSPlainTextEdit->setReadOnly(false);
    }

    connect(useDefaultCheckBox, SIGNAL(stateChanged(int)),
            this, SLOT(checkStateChanged()));
}

StylesPage::~StylesPage()
{
    delete cssHighLighter;
    cssHighLighter = NULL;
    delete ui;
}

void StylesPage::saveConfig()
{
    conf->setMarkdownCSS(useDefaultCheckBox->isChecked(), customCSSPlainTextEdit->toPlainText());
}

void StylesPage::checkStateChanged()
{
    customCSSPlainTextEdit->setReadOnly(useDefaultCheckBox->isChecked());
}
