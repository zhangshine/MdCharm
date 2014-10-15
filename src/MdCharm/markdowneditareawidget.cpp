#include <QPlainTextEdit>

#ifdef QT_V5
#include <QtWebKitWidgets>
#else
#include <QtWebKit/QWebFrame>
#include <QtWebKit/QWebPage>
#endif

#include <QSplitter>
#include <QResizeEvent>
#include <QTextStream>
#include <QPrinter>
#include <QTextDocumentWriter>
#include <QTextDocument>
#include <QDir>
#include <QtGui>
#include <QWebElement>

#include "markdowneditareawidget.h"
#include "markdowntohtml.h"
#include "resource.h"
#include "utils.h"
#include "util/zip/zipwriter.h"
#include "util/odt/odtwriter.h"
#include "baseeditor/markdowneditor.h"
#include "configuration.h"
#include "util/gui/findandreplace.h"
#include "util/gui/insertlinkorpicturedialog.h"
#include "util/gui/insertcodedialog.h"
#include "util/syntax/hightlighter.h"
#include "mdcharmform.h"
#include "dock/projectdockwidget.h"
#include "basewebview/markdownwebview.h"

//------------------MarkdownWebkitHandler---------------------------------------

MarkdownWebkitHandler::MarkdownWebkitHandler()
{
    m_domReady = false;
}

void MarkdownWebkitHandler::domReady()
{
    m_domReady = true;
    emit initContent();
}

bool MarkdownWebkitHandler::isDomReady()
{
    return m_domReady;
}

MarkdownWebkitHandler::~MarkdownWebkitHandler()
{
}

//------------------MarkdownEditAreaWidget--------------------------------------

MarkdownEditAreaWidget::MarkdownEditAreaWidget(MdCharmForm *mainForm, const QString &filePath, const QUrl &baseUrl) :
    EditAreaWidget(filePath,
                   AllowSaveAs|AllowSelectAll|AllowExportToHtml|AllowExportToODT
                   |AllowExportToPdf|AllowPrint|AllowPreview|AllowFind|AllowSplit),
    mainForm(mainForm)
{
    inited = false;
//    lastRevision = -2;
    this->baseUrl = baseUrl;
    em.setEditorType(EditorModel::MARKDOWN);

    initGui();
    initContent(filePath);
    initConfiguration();
    initPreviewerMatter();
    initSignalsAndSlots();
}

void MarkdownEditAreaWidget::initPreviewerMatter()
{
    connect(previewer, SIGNAL(linkClicked(QUrl)),
            this, SLOT(openUrl(QUrl)));
    markdownWebkitHandler = new MarkdownWebkitHandler();
    addJavascriptObject();//warning:add before setHtml!!!!!!!!!!!!!!!!!!!!!!!!!!
    QObject::connect(previewer->page()->mainFrame(),SIGNAL(javaScriptWindowObjectCleared()),
                     this, SLOT(addJavascriptObject()));
    QObject::connect(previewer->page(), SIGNAL(linkHovered(QString,QString,QString)),
                     this, SIGNAL(showStatusMessage(QString)));
    initHtmlEngine();
}

void MarkdownEditAreaWidget::initHtmlEngine()
{
    QFile htmlTemplate(":/markdown/markdown.html");
    if(!htmlTemplate.open(QIODevice::ReadOnly))
    {
        Utils::showFileError(htmlTemplate.error(), ":/markdown/markdown.html");
        return;
    }
    QString htmlContent = htmlTemplate.readAll();
    htmlTemplate.close();

    std::string textResult = convertMarkdownToHtml();

    previewer->setHtml(htmlContent.arg(conf->getMarkdownCSS())
                       .arg("<script type=\"text/javascript\" src=\"qrc:/jquery.js\"></script>")
                       .arg("<script type=\"text/javascript\" src=\"qrc:/markdown/markdown.js\"></script>")
                       .arg(QString::fromUtf8(textResult.c_str(), textResult.length())),
                       baseUrl);
}

void MarkdownEditAreaWidget::initGui()
{
    splitter = new QSplitter(Qt::Horizontal, this);
#ifdef Q_OS_WIN
    splitter->setStyleSheet("QSplitter::handle{background-color: gray;}");
    splitter->setHandleWidth(1);
#endif
    editor = new MarkdownEditor(this);
    editorScrollBar = editor->verticalScrollBar();
    splitter->addWidget(editor);
    previewer = new MarkdownWebView(this);
    previewer->setAcceptDrops(false);
    previewer->setBaseSize(editor->baseSize().width(),previewer->baseSize().height());
    splitter->addWidget(previewer);
    findAndReplaceWidget = new FindAndReplace(this);
    findAndReplaceWidget->hide();
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0,0,0,0);
    mainLayout->setSpacing(0);
    mainLayout->addWidget(splitter);
    mainLayout->addWidget(findAndReplaceWidget);
    setLayout(mainLayout);
}

void MarkdownEditAreaWidget::initConfiguration()
{
    previewer->page()->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);//make qwebkit not load the link

    if (conf->isDisplayLineNumber())
        editor->enableDisplayLineNumber();
    else
        editor->disableDisplayLineNumber();
    if (conf->isHighlightCurrentLine())
        editor->enableHighlightCurrentLine();
    else
        editor->disableHighlightCurrentLine();
    editor->setWordWrapMode(conf->isEnableTextWrapping() ?  QTextOption::WrapAtWordBoundaryOrAnywhere:QTextOption::NoWrap);
    QFont font(conf->getFontFamily(), conf->getFontSize());
    editor->document()->setDefaultFont(font);

    editor->setTabStopWidth(conf->getTabSize()*QFontMetrics(editor->document()->defaultFont()).width(" "));
    switch(conf->getPreviewOption()){
        case MdCharmGlobal::WriteMode:
            previewer->setHidden(true);
            break;
        case MdCharmGlobal::ReadMode:
            editor->setHidden(true);
            break;
        default:
            break;
    }
}

void MarkdownEditAreaWidget::initContent(const QString &filePath)
{
    doc = QSharedPointer<QTextDocument>(new QTextDocument(NULL));
    QAbstractTextDocumentLayout *layout = new QPlainTextDocumentLayout(doc.data());
    doc->setDocumentLayout(layout);
    editor->setDocument(doc.data());
    highlighter = new MarkdownHighLighter(doc.data());
    //deal file content
    if(filePath.isEmpty())
    {
        fm->setEncodingFormatName(conf->getDefaultEncoding());//default encoding
        setModified(false);
        return;
    }
    QFile openFile(filePath);
    if (!openFile.open(QIODevice::ReadOnly))
    {
        Utils::showFileError(openFile.error(), filePath);
        return;
    }
    QTextStream openFileStream(&openFile);
    //autodetected
    QString fileContent = openFileStream.readAll();
    QByteArray cn = openFileStream.codec()->name();
    fm->setEncodingFormatName(cn);
    if(cn.startsWith("UTF"))//UTF8 UTF16LE UTF16BE
        fm->setHasBom(true);
    if(cn==QByteArray("System"))
    {//check if it is utf8 without bom
        openFile.seek(0);
        if(Utils::isUtf8WithoutBom(openFile.readAll()))
        {
            openFileStream.setCodec("UTF-8");
            fm->setEncodingFormatName("UTF-8");
            fm->setHasBom(false);
            openFileStream.seek(0);
            fileContent = openFileStream.readAll();
        }
    }
    if(cn.isEmpty())
    {
        fm->setEncodingFormatName(conf->getDefaultEncoding());
        fm->setHasBom(true);
    }
    doc->setPlainText(fileContent);
    doc->setModified(false);
    openFile.close();
}

void MarkdownEditAreaWidget::initSignalsAndSlots()
{
    if(conf->getPreviewOption()==MdCharmGlobal::WriteRead)
        connect(editor, SIGNAL(textChanged()),
                         this, SLOT(parseMarkdown()));
    if(conf->isSyncScrollbar()){
        connect(editorScrollBar, SIGNAL(valueChanged(int)),
                this, SLOT(scrollPreviewTo(int)));
        connect(editor, SIGNAL(textChanged()),
                this, SLOT(scrollPreviewTo()));
    }
    connect(editor, SIGNAL(modificationChanged(bool)),
            this, SIGNAL(updateActions()));

    connect(editor, SIGNAL(cursorPositionChanged()),
                     this, SLOT(cursorPositionChanged()));

    connect(editor, SIGNAL(copyAvailable(bool)),
                     this, SLOT(setCopyAvaliable(bool)));
    connect(editor, SIGNAL(undoAvailable(bool)),
            this, SIGNAL(updateActions()));
    connect(editor, SIGNAL(redoAvailable(bool)),
            this, SIGNAL(updateActions()));
    connect(editor, SIGNAL(overWriteModeChanged()),
                     this, SLOT(overWriteModeChanged()));
    connect(editor, SIGNAL(textChanged()), this, SIGNAL(textChanged()));

    connect(findAndReplaceWidget, SIGNAL(findText(QString,QTextDocument::FindFlags,bool, bool)),
                     editor, SLOT(findAndHighlightText(QString, QTextDocument::FindFlags,bool, bool)));
    connect(findAndReplaceWidget, SIGNAL(findHide()),
                     editor, SLOT(findFinished()));
    connect(findAndReplaceWidget, SIGNAL(findHide()),
                     this, SLOT(setFocusEditor()));
    connect(findAndReplaceWidget, SIGNAL(findHide()),
                     this, SLOT(hideFind()));
    connect(findAndReplaceWidget, SIGNAL(findNext(QString,QTextDocument::FindFlags,bool)),
                     editor, SLOT(findFirstOccurrance(QString,QTextDocument::FindFlags,bool)));
    connect(findAndReplaceWidget, SIGNAL(findPrevious(QString,QTextDocument::FindFlags,bool)),
                     editor, SLOT(findFirstOccurrance(QString,QTextDocument::FindFlags,bool)));
    connect(findAndReplaceWidget, SIGNAL(replace(QString)),
                     editor, SLOT(replace(QString)));
    connect(findAndReplaceWidget, SIGNAL(replaceAll(QString,QString,QTextDocument::FindFlags,bool)),
                     editor, SLOT(replaceAll(QString,QString,QTextDocument::FindFlags,bool)));
    connect(editor, SIGNAL(focusInSignal()), this, SIGNAL(focusInSignal()));
}

void MarkdownEditAreaWidget::parseMarkdown()
{
//    if (lastRevision == editor->document()->revision())
//        return;
//    lastRevision = editor->document()->revision();
    std::string textResult = convertMarkdownToHtml();
    previewer->page()->mainFrame()->findFirstElement("body").setInnerXml(QString::fromUtf8(textResult.c_str(), textResult.length()));
}

void MarkdownEditAreaWidget::reFind()
{
    findAndReplaceWidget->resendFindTextSignal();
}

void MarkdownEditAreaWidget::addJavascriptObject()
{
    previewer->page()->mainFrame()
            ->addToJavaScriptWindowObject("markdownWebkitHandler", markdownWebkitHandler);
}

void MarkdownEditAreaWidget::setText(const QString &text)
{
    editor->setPlainText(text);
}

QString MarkdownEditAreaWidget::getText()
{
    return editor->toPlainText();
}

void MarkdownEditAreaWidget::resizeEvent(QResizeEvent *event)
{
    const QSize size = event->size();
    if(!inited && size.width()>0)
    {
        const int halfSize = size.width()/2;
        QList<int> widgetSize;
        widgetSize.append(halfSize);
        widgetSize.append(halfSize);
        splitter->setSizes(widgetSize);
        inited = true;
    }
    EditAreaWidget::resizeEvent(event);
}

void MarkdownEditAreaWidget::exportToPdf(const QString &filePath)
{
    parseMarkdown();
    QPrinter printer;
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(filePath);
    printer.setCreator("MdCharm(http://www.mdcharm.com/)");
    previewer->print(&printer);
}

void MarkdownEditAreaWidget::exportToODT(const QString &filePath)
{
    std::string textResult = convertMarkdownToHtml();
    QTextDocument textDocument;
    Configuration *conf = Configuration::getInstance();
    QFont font(conf->getFontFamily(), conf->getFontSize());
    textDocument.setDefaultFont(font);
    textDocument.setHtml(QString::fromUtf8(textResult.c_str(), textResult.length()));
//    QTextDocumentWriter writer(filePath, QByteArray("odf"));
//    writer.write(&textDocument);
    ODTWriter odtWriter(textDocument, filePath);
    odtWriter.writeAll();
}

void MarkdownEditAreaWidget::exportToHtml(const QString &filePath)
{
    QFile htmlTemplate(":/markdown/markdown.html");
    if(!htmlTemplate.open(QIODevice::ReadOnly))
    {
        Utils::showFileError(htmlTemplate.error(), ":/markdown/markdown.html");
        return;
    }
    QString htmlContent = htmlTemplate.readAll();
    htmlTemplate.close();

    std::string textResult = convertMarkdownToHtml();

    htmlContent = htmlContent.arg(conf->getMarkdownCSS())
                       .arg("")
                       .arg("")
                       .arg(QString::fromUtf8(textResult.c_str(), textResult.length()));
    Utils::saveFile(filePath, htmlContent.toUtf8());
}

void MarkdownEditAreaWidget::printContent()
{
    QPrinter printer(QPrinter::HighResolution);
    printer.setDocName(fm->getFileName());

    QPrintDialog dialog(&printer, this);
    dialog.setWindowTitle(tr("Print Document"));
    if(dialog.exec() == QDialog::Accepted)
    {
        editor->print(&printer);
    }
}

void MarkdownEditAreaWidget::printPreview()
{
    QPrinter printer(QPrinter::HighResolution);
    printer.setDocName(fm->getFileName());
    QPrintPreviewDialog ppd(&printer, this, Qt::WindowMaximizeButtonHint);
    connect(&ppd, SIGNAL(paintRequested(QPrinter*)), this, SLOT(paintRequestSlot(QPrinter*)));
    ppd.exec();
}

bool MarkdownEditAreaWidget::saveFile()
{
    QString fileFullPath = fm->getFileFullPath();
    if (fileFullPath.isEmpty())
    {
        QStringList filterList = conf->getFileOpenFilters(Configuration::MarkdownFile);
        QString dir;
        QVariant var = conf->getLastStateValue("MarkdownEditArea_SaveFile");
        if(var.isValid() && var.canConvert(QVariant::String))
            dir = var.toString();
        fileFullPath = Utils::getSaveFileName(QString::fromLatin1(".md"), this, QString::fromLatin1("Save"), //TODO: use recommand suffix instead of .md
                                              dir.isEmpty() ? QDir::homePath() : dir, filterList.join(";;"));
        if (fileFullPath.isEmpty())
            return false;
        conf->setLastStateValue("MarkdownEditArea_SaveFile", QFileInfo(fileFullPath).absoluteDir().absolutePath());
        fm->setFileFullPath(fileFullPath);
        emit addToRecentFileList(fileFullPath);
    }
    bool bom = Utils::calculateBom(fm->isHasBom(), fm->getEncodingFormatName(),
                                   conf->getUtf8BOMOptions());
    bool isSaveSuccess = Utils::saveFile(fileFullPath,
                                         Utils::encodeString(editor->toPlainText(), fm->getEncodingFormatName(), bom));
    if(isSaveSuccess)
        setModified(false);
    return isSaveSuccess;
}

void MarkdownEditAreaWidget::saveFileAs()
{
    QStringList filterList = conf->getFileOpenFilters(Configuration::MarkdownFile);
    QString dir;
    QVariant var = conf->getLastStateValue("MarkdownEditArea_SaveFileAs");
    if(var.isValid() && var.canConvert(QVariant::String))
        dir = var.toString();
    QString fileFullPath = Utils::getSaveFileName(QString::fromLatin1(".md"), this, QString::fromLatin1("Save as"), //TODO: use recommand suffix instead of .md
                                                  dir.isEmpty() ? QDir::homePath() : dir, filterList.join(";;"));
    if (fileFullPath.isEmpty())
        return;
    conf->setLastStateValue("MarkdownEditArea_SaveFileAs", QFileInfo(fileFullPath).absoluteDir().absolutePath());
    bool bom = Utils::calculateBom(fm->isHasBom(), fm->getEncodingFormatName(),
                                   conf->getUtf8BOMOptions());
    Utils::saveFile(fileFullPath, Utils::encodeString(editor->toPlainText(), fm->getEncodingFormatName(), bom));
    fm->setFileFullPath(fileFullPath);
    setModified(false);
}

void MarkdownEditAreaWidget::switchPreview(int type)
{
    disconnect(editor, SIGNAL(textChanged()), this, SLOT(parseMarkdown()));
    switch(type){
        case MdCharmGlobal::WriteMode:
            previewer->setVisible(false);
            editor->setVisible(true);
            break;
        case MdCharmGlobal::WriteRead:
            previewer->setVisible(true);
            editor->setVisible(true);
            connect(editor, SIGNAL(textChanged()), this, SLOT(parseMarkdown()));
            parseMarkdown();
            break;
        case MdCharmGlobal::ReadMode:
            editor->setVisible(false);
            previewer->setVisible(true);
            parseMarkdown();
            break;
        default:
            break;
    }
}

void MarkdownEditAreaWidget::changeSyncScrollbarSetting(bool sync)
{
    disconnect(editorScrollBar, SIGNAL(valueChanged(int)),
               this, SLOT(scrollPreviewTo(int)));
    disconnect(editor, SIGNAL(cursorPositionChanged()),
               this, SLOT(scrollPreviewTo()));
    if(sync)
    {
        connect(editorScrollBar, SIGNAL(valueChanged(int)),
                this, SLOT(scrollPreviewTo(int)));
        connect(editor, SIGNAL(cursorPositionChanged()),
                this, SLOT(scrollPreviewTo()));
    }
}

void MarkdownEditAreaWidget::reloadFile()
{
    FileModel fm = getFileModel();
    QFile openFile(fm.getFileFullPath());
    if(!openFile.open(QIODevice::ReadOnly))
    {
        Utils::showFileError(openFile.error(), fm.getFileFullPath());
        return;
    }
    QTextStream openFileStream(&openFile);
    openFileStream.setCodec(QTextCodec::codecForName(fm.getEncodingFormatName().toLatin1()));
    setText(openFileStream.readAll());
    openFile.close();
    setModified(false);//after reload file, the file status is not modified
}

int MarkdownEditAreaWidget::getCurrentMaxBlockCount()
{
    return editor->blockCount();
}

void MarkdownEditAreaWidget::gotoLine(int line)
{
    QTextCursor textCursor = editor->textCursor();
    int current = textCursor.blockNumber()+1;
    QTextCursor::MoveOperation mo;
    if(current>line)
    {
        mo = QTextCursor::PreviousBlock;
    } else if (current<line) {
        mo = QTextCursor::NextBlock;
    } else {
        return;
    }
    textCursor.movePosition(mo,QTextCursor::MoveAnchor, abs(current-line));
    editor->setTextCursor(textCursor);
}

const StateModel MarkdownEditAreaWidget::getState()
{
    StateModel sm;
    sm.setFirstVisibleLine(editor->firstVisibleLineNumber());
    QTextCursor cursor = editor->textCursor();
    sm.setSelectionStart(cursor.selectionStart());
    sm.setSelectionEnd(cursor.selectionEnd());
    sm.setVerticalScrllBarCurrentValue(editorScrollBar->value());
    sm.setVerticalScrollBarMaxValue(editorScrollBar->maximum());
    sm.setFileFullPath(fm->getFileFullPath());
    return sm;
}

void MarkdownEditAreaWidget::restoreFileState(const StateModel &sm)
{
    QTextCursor cursor = editor->textCursor();
    cursor.setPosition(sm.getSelectionStart());
    if(sm.getSelectionStart()!=sm.getSelectionEnd()){
        cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, sm.getSelectionEnd()-sm.getSelectionStart());
    }
    editor->setTextCursor(cursor);

    int firstVisibleLine = sm.getFirstVisibleLine();
    if(firstVisibleLine > 1 && editorScrollBar->isVisible() &&
            firstVisibleLine<=editor->blockCount()){
        if((sm.getVerticalScrollBarMaxValue()==editorScrollBar->maximum() || sm.getVerticalScrollBarMaxValue()==(editorScrollBar->maximum()+1))&&
                sm.getVerticalScorllBarCurrentValue()<=editorScrollBar->maximum())
        {
            editorScrollBar->setValue(sm.getVerticalScorllBarCurrentValue());
        } else {
            gotoLine(firstVisibleLine);
            QTextBlock block = editor->textCursor().block();
            while(editor->firstVisibleLineNumber()<firstVisibleLine && block.isValid()){
                gotoLine(block.blockNumber());
                block = block.next();
            }
        }
    }
}

void MarkdownEditAreaWidget::cut()
{
    editor->cut();
}

void MarkdownEditAreaWidget::copy()
{
    editor->copy();
}

void MarkdownEditAreaWidget::paste()
{
    editor->paste();
}

void MarkdownEditAreaWidget::selectAll()
{
    editor->selectAll();
}

bool MarkdownEditAreaWidget::isModified()
{
    return document()->isModified();
}

bool MarkdownEditAreaWidget::isUndoAvailable()
{
    return document()->isUndoAvailable();
}

bool MarkdownEditAreaWidget::isRedoAvailable()
{
    return document()->isRedoAvailable();
}

void MarkdownEditAreaWidget::setModified(bool isModi)
{
    document()->setModified(isModi);
}

void MarkdownEditAreaWidget::redo()
{
    editor->redo();
}

void MarkdownEditAreaWidget::undo()
{
    editor->undo();
}

void MarkdownEditAreaWidget::updateConfiguration()
{
    previewer->page()->setLinkDelegationPolicy(QWebPage::DelegateExternalLinks);//make qwebkit not load the link

    if (conf->isDisplayLineNumber())
        editor->enableDisplayLineNumber();
    else
        editor->disableDisplayLineNumber();
    if (conf->isHighlightCurrentLine())
        editor->enableHighlightCurrentLine();
    else
        editor->disableHighlightCurrentLine();
    if (conf->isCheckSpell())
        editor->enableSpellCheck();
    else
        editor->disableSpellCheck();
    editor->setWordWrapMode(conf->isEnableTextWrapping() ?  QTextOption::WrapAtWordBoundaryOrAnywhere:QTextOption::NoWrap);
    QFont font(conf->getFontFamily(), conf->getFontSize());
    editor->document()->setDefaultFont(font);

    editor->setTabStopWidth(conf->getTabSize()*QFontMetrics(editor->document()->defaultFont()).width(" "));
//    switchPreview(conf->getPreviewOption());
    changeSyncScrollbarSetting(conf->isSyncScrollbar());
    initHtmlEngine();
}

void MarkdownEditAreaWidget::setFocusEditor()
{
    editor->setFocus();
}

void MarkdownEditAreaWidget::setFocusFinder()
{
    findAndReplaceWidget->setFocusTextEdit();
}

void MarkdownEditAreaWidget::cursorPositionChanged()
{
    QTextCursor tc = editor->textCursor();
    em.setCurrentLineNumber(tc.blockNumber()+1);
    em.setCurrentColumnNumber(tc.columnNumber()+1);
    emit updateStatusBar();
}

void MarkdownEditAreaWidget::overWriteModeChanged()
{
    em.overWriteChanged();
    editor->setOverwriteMode(em.isOverWrite());
    emit updateStatusBar();
}

void MarkdownEditAreaWidget::showFind()
{
    findAndReplaceWidget->show();
    em.setFindVisible(true);
    disconnect(editor, SIGNAL(textChanged()),
               this, SLOT(reFind()));
    connect(editor, SIGNAL(textChanged()),
                     this, SLOT(reFind()));
    QTextCursor cursor = editor->textCursor();
    if(cursor.hasSelection()){
        findAndReplaceWidget->setFindText(cursor.selectedText());
    } else {
        QClipboard *clipboard = QApplication::clipboard();
        const QString clipText = clipboard->text();
        if(!clipText.trimmed().isEmpty())
            findAndReplaceWidget->setFindText(clipText);
    }
    emit updateActions();
}

void MarkdownEditAreaWidget::dealMarkdownMenuAction(int type)
{
    switch(type)
    {
        case MdCharmGlobal::ShortcutBold:
            editor->boldText();
            break;
        case MdCharmGlobal::ShortcutItalic:
            editor->italicText();
            break;
        case MdCharmGlobal::ShortcutQuoteText:
            editor->quoteText();
            break;
        case MdCharmGlobal::ShortcutStrikeThrough:
            editor->strikeThroughText();
            break;
        case MdCharmGlobal::ShortcutTabBlockText:
            editor->tabText();
            break;
        case MdCharmGlobal::ShortcutShiftTab:
            editor->untabText();
            break;
        case MdCharmGlobal::ShortcutInsertLink:
        case MdCharmGlobal::ShortcutInsertPicture:
            insertLinkOrPicture(type);
            break;
        case MdCharmGlobal::ShortcutInsertCode:
            insertCode();
            break;
        default:
            break;
    }
}

void MarkdownEditAreaWidget::disableSpellCheck()
{
    editor->disableSpellCheck();
}

void MarkdownEditAreaWidget::enableSpellCheck()
{
    editor->enableSpellCheck();
}

QTextDocument* MarkdownEditAreaWidget::document()
{
    return editor->document();
}

QString MarkdownEditAreaWidget::getProDir()
{
    QString proDir = mainForm->getProjectDockWidget()->getProjectDir();
    if(proDir.isEmpty() || !fm->getFileFullPath().startsWith(proDir))
        proDir = QFileInfo(fm->getFileFullPath()).absolutePath();
    return proDir;
}

void MarkdownEditAreaWidget::insertLinkOrPicture(int type)
{
    InsertLinkOrPictureDialog insertPLDialog(type, getProDir(), this);
    if(QDialog::Accepted==insertPLDialog.exec()){
        editor->insertLinkOrPicuture(type, insertPLDialog.getText(),
                                     insertPLDialog.getUrl(),
                                     insertPLDialog.getTitle(),
                                     insertPLDialog.getWidth(),
                                     insertPLDialog.getHeight());
    }
}

void MarkdownEditAreaWidget::insertCode()
{
    if(conf->getMarkdownEngineType()==MarkdownToHtml::MultiMarkdown){
        editor->insertCode(QString());
    } else {
        InsertCodeDialog insertCodeDialog(this);
        if(insertCodeDialog.exec()==QDialog::Accepted && !insertCodeDialog.getCodeType().isEmpty()){
            editor->insertCode(insertCodeDialog.getCodeType());
        }
    }
}

void MarkdownEditAreaWidget::hideFind()
{
    findAndReplaceWidget->hide();
    disconnect(editor, SIGNAL(textChanged()),
               this, SLOT(reFind()));
    em.setFindVisible(false);
    emit updateActions();
}

void MarkdownEditAreaWidget::findNext()
{
    findAndReplaceWidget->resendFindNextSignal();
}

void MarkdownEditAreaWidget::findPrevious()
{
    findAndReplaceWidget->resendFindPreviousSignal();
}

void MarkdownEditAreaWidget::scrollPreviewTo(int value)//Vertical scrollbar visible=true
{
    float maxEdit = editorScrollBar->maximum();
    int previewMax = previewer->page()->mainFrame()->scrollBarMaximum(Qt::Vertical);
    previewer->page()->mainFrame()->setScrollBarValue(Qt::Vertical, value/maxEdit*previewMax);
}

void MarkdownEditAreaWidget::scrollPreviewTo()//Vertical scrollbar visible=false
{
    int blockCount = editor->blockCount();
    int curBlock = editor->textCursor().blockNumber()+1;//FIXME: crashrpt 60e48fe9-6bfc-43cd-afac-002f89817ead
    int previewMax = previewer->page()->mainFrame()->scrollBarMaximum(Qt::Vertical);
    if(!editorScrollBar->isVisible() || blockCount==curBlock)
        previewer->page()->mainFrame()->setScrollBarValue(Qt::Vertical, curBlock/blockCount*previewMax);

}

void MarkdownEditAreaWidget::openUrl(const QUrl &url)
{
    if(url.toString(QUrl::RemoveQuery|QUrl::RemoveFragment)==previewer->url().toString(QUrl::RemoveQuery|QUrl::RemoveFragment)){
        if(!url.fragment().isEmpty())
            previewer->page()->mainFrame()->scrollToAnchor(url.fragment());
        return;
    }
    QDesktopServices::openUrl(url);
}

void MarkdownEditAreaWidget::paintRequestSlot(QPrinter *printer)
{
    editor->print(printer);
}

MarkdownEditAreaWidget::~MarkdownEditAreaWidget()
{
    markdownWebkitHandler->deleteLater();
}

std::string MarkdownEditAreaWidget::convertMarkdownToHtml()
{
    QByteArray content = editor->toPlainText().toUtf8();
    std::string textResult;
    MarkdownToHtml::translateMarkdownToHtml(conf->getMarkdownEngineType(), content.data(), content.length(), textResult);
    return textResult;
}

void MarkdownEditAreaWidget::jumpToPreviewAnchor(const QString &anchor)
{
    previewer->page()->currentFrame()->scrollToAnchor(anchor);
}

//-------------------------------- Clone ---------------------------------------
MarkdownEditAreaWidget::MarkdownEditAreaWidget(MarkdownEditAreaWidget &src) :
    EditAreaWidget(src)
{
    mainForm = src.mainForm;
    inited = false;
    baseUrl = src.baseUrl;
    doc = QSharedPointer<QTextDocument>(src.doc);
    em.setEditorType(EditorModel::MARKDOWN);

    initGui();
    editor->setDocument(doc.data());
    initConfiguration();
    initPreviewerMatter();
    initSignalsAndSlots();
}

EditAreaWidget* MarkdownEditAreaWidget::clone()
{
    Q_ASSERT(isEditActionOptionEnabled(AllowSplit));
    return new MarkdownEditAreaWidget(*(this));
}
