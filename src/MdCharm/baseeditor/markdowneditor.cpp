#include <QTextBlock>
#include <QMenu>
#include <QClipboard>
#include <QApplication>
#include <QDragEnterEvent>
#include <QMimeData>
#include <QUrl>
#include <QProcess>

#include "markdowneditor.h"
#include "markdowntohtml.h"
#include "configuration.h"
#include "markdownautocompleter.h"
#include "resource.h"
#include "markdowneditareawidget.h"
#include "util/spellcheck/spellchecker.h"

MarkdownEditor::MarkdownEditor(MarkdownEditAreaWidget *parent):
    BaseEditor(parent),
    parent(parent)
{
    mdcharmGlobalInstance = MdCharmGlobal::getInstance();
    copyAsHtmlAction = new QAction(tr("Copy as Html"), this);
    autoCompleter = new MarkdownAutoCompleter(this);
    connect(copyAsHtmlAction, SIGNAL(triggered()), this, SLOT(copyAsHtmlSlot()));
    setFrameShape(QTextEdit::NoFrame);
    setAcceptDrops(true);
}

void MarkdownEditor::keyPressEvent(QKeyEvent *event)
{
    if (event->key()==Qt::Key_Tab){
        tabText();
        event->accept();
        return;
    } else if (event == QKeySequence::InsertParagraphSeparator) {
        QTextCursor tc = textCursor();
        int newBlocks = autoCompleter->paragraphSeparatorAboutToBeInserted(tc);
        if(newBlocks!=0){
            event->accept();
            ensureCursorVisible();//when at the end, cursor may not visible
            return;
        }
        //WARNING!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
        //if newBlocks==0 must not return and accept
    } else if( (event->modifiers() & (Qt::ControlModifier|Qt::AltModifier)) != Qt::ControlModifier
               && conf->isAutoPair()) {

        QTextCursor cursor = textCursor();
        QString text = event->text();
        const QString autoText = autoCompleter->autoComplete(cursor, text);
        if(!autoText.isEmpty()){
            event->accept();
            cursor.beginEditBlock();
            cursor.insertText(text);
            int pos = cursor.position();
            cursor.insertText(autoText);
            cursor.setPosition(pos);
            cursor.endEditBlock();
            setTextCursor(cursor);
            return;
        }
    }

    BaseEditor::keyPressEvent(event);
}

void MarkdownEditor::contextMenuEvent(QContextMenuEvent *e)
{
    // copy as Html
    e->accept();
    QMenu *menu = createStandardContextMenu();
    menu->addSeparator();
    QAction* spellCheckStatusAction = new QAction(tr("Check spelling"), menu);
    spellCheckStatusAction->setCheckable(true);
    spellCheckStatusAction->setChecked(!spellCheckLanguage.isEmpty());
    menu->addAction(spellCheckStatusAction);
    QList<QAction *> spellCheckLanguageActions = addSpellCheckLanguageActions(menu);
    menu->addSeparator();
    copyAsHtmlAction->setEnabled(textCursor().hasSelection());
    menu->addAction(copyAsHtmlAction);
    QList<QAction *> spellCheckActions;
    if(e->reason()==QContextMenuEvent::Mouse&&!textCursor().hasSelection()){
        setTextCursor(cursorForPosition(e->pos()));
        spellCheckActions = addSpellCheckActions(menu);
    }
    QAction *selected = menu->exec(e->globalPos());
    if(selected){
        if(spellCheckActions.indexOf(selected)!=-1){
            replaceTextInCurrentCursor(selected->text());
        } else if(selected==spellCheckStatusAction) {
            if(selected->isChecked()){//enable spell check
                spellCheckLanguage = conf->getSpellCheckLanguage();
                if(conf->getSpellCheckLanguage().isEmpty())
                    spellCheckLanguage = conf->getAllAvailableSpellCheckDictNames().first();
                enableSpellCheck();
            } else {//disable spell check
                spellCheckLanguage.clear();
                disableSpellCheck();
            }
        } else if(spellCheckLanguageActions.indexOf(selected)!=-1 &&
                  selected->data().canConvert(QVariant::String) &&
                  !selected->data().toString().isEmpty() &&
                  selected->data().toString()!=spellCheckLanguage){
            disableSpellCheck();
            spellCheckLanguage = selected->data().toString();
            enableSpellCheck();
        }
    }
    menu->deleteLater();
}

void MarkdownEditor::dragEnterEvent(QDragEnterEvent *e)
{
    if (e->mimeData()->hasUrls()||e->mimeData()->hasFormat("text/uri-list")) {
        e->acceptProposedAction();
        e->accept();
    } else {
        BaseEditor::dragEnterEvent(e);
    }
}

void MarkdownEditor::dragMoveEvent(QDragMoveEvent *e)
{
    if (e->mimeData()->hasUrls()||e->mimeData()->hasFormat("text/uri-list")) {
        e->acceptProposedAction();
        e->accept();
    } else {
        BaseEditor::dragMoveEvent(e);
    }
}

void MarkdownEditor::dropEvent(QDropEvent *e)
{
    if(e->mimeData()->hasUrls()||e->mimeData()->hasFormat("text/uri-list")){
        QStringList ignoredUrls;
        setTextCursor(cursorForPosition(e->pos()));
        QDir current(parent->getProDir());
        foreach (QUrl url, e->mimeData()->urls()) {
            QString urlStr = url.toString();
            QFileInfo fileInfo(urlStr);
            QString ext = fileInfo.suffix();
            if(Utils::ImageExts.contains(ext)){//Images
                insertLinkOrPicuture(MdCharmGlobal::ShortcutInsertPicture, fileInfo.fileName(), current.relativeFilePath(url.toLocalFile()));
            } else if (!url.isLocalFile())//external link
                insertLinkOrPicuture(MdCharmGlobal::ShortcutInsertLink, urlStr, urlStr);
            else
                ignoredUrls.append(url.toLocalFile());
        }
        if(!ignoredUrls.isEmpty()){
            QProcess::startDetached(qApp->applicationFilePath(), ignoredUrls);
        }
        e->accept();
    } else {
        BaseEditor::dropEvent(e);
    }
}

void MarkdownEditor::replaceTextInCurrentCursor(const QString &text)
{
    QTextCursor cursor = textCursor();
    cursor.select(QTextCursor::WordUnderCursor);
    cursor.insertText(text);
}

void MarkdownEditor::tabText()
{
    QTextCursor cursor = textCursor();
    QString text;
    if(conf->isUseWhiteSpaceInsteadOfTab())
        text.append("    ");//4*space
    else
        text.append('\t');
    if(cursor.hasSelection()){
        int start = cursor.selectionStart();
        QTextBlock block = document()->findBlock(cursor.selectionStart());
        QTextBlock endBlock = document()->findBlock(cursor.selectionEnd()).next();
        cursor.clearSelection();
        cursor.beginEditBlock();
        cursor.setPosition(start);
        cursor.movePosition(QTextCursor::StartOfBlock);
        do {
            cursor.insertText(text);
            cursor.movePosition(QTextCursor::NextBlock);
            block = block.next();
        } while(block.isValid() && block != endBlock);
        cursor.endEditBlock();
    } else {
        cursor.insertText(text);
    }
}

void MarkdownEditor::italicText()
{
    //italic
    QTextCursor cursor = textCursor();
    if(cursor.hasSelection())
    {
        int start = cursor.selectionStart();
        int currentPos = cursor.position();
        int end = cursor.selectionEnd();
        //add '*'
        cursor.beginEditBlock();
        cursor.clearSelection();
        cursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, end-currentPos);
        cursor.insertText("*");
        cursor.movePosition(QTextCursor::Left, QTextCursor::MoveAnchor, abs(end-start)+1);
        cursor.insertText("*");
        cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, abs(end-start));
        cursor.endEditBlock();
        setTextCursor(cursor);
    } else {
        cursor.beginEditBlock();
        cursor.insertText("**");
        cursor.movePosition(QTextCursor::PreviousCharacter, QTextCursor::MoveAnchor);
        cursor.endEditBlock();
        setTextCursor(cursor);
    }
}

void MarkdownEditor::boldText()
{
    //bold
    QTextCursor cursor = textCursor();
    if(cursor.hasSelection())
    {
        int start = cursor.selectionStart();
        int currentPos = cursor.position();
        int end = cursor.selectionEnd();
        QString text("**");
        //add "**"
        cursor.beginEditBlock();
        cursor.clearSelection();
        cursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, end-currentPos);
        cursor.insertText(text);
        cursor.movePosition(QTextCursor::Left, QTextCursor::MoveAnchor, abs(end-start)+text.length());
        cursor.insertText(text);
        cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, abs(end-start));
        cursor.endEditBlock();
        setTextCursor(cursor);
    } else {
        cursor.beginEditBlock();
        cursor.insertText("****");
        cursor.movePosition(QTextCursor::Left, QTextCursor::MoveAnchor, 2);
        cursor.endEditBlock();
        setTextCursor(cursor);
    }
}

void MarkdownEditor::strikeThroughText()
{
    //strike through
    QTextCursor cursor = textCursor();
    if(cursor.hasSelection()){
        int start = cursor.selectionStart();
        int currentPos = cursor.position();
        int end = cursor.selectionEnd();
        QString text("~~");
        //add "~~"
        cursor.beginEditBlock();
        cursor.clearSelection();
        cursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, end-currentPos);
        cursor.insertText(text);
        cursor.movePosition(QTextCursor::Left, QTextCursor::MoveAnchor, abs(end-start)+text.length());
        cursor.insertText(text);
        cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, abs(end-start));
        cursor.endEditBlock();
        setTextCursor(cursor);
    } else {
        cursor.beginEditBlock();
        cursor.insertText("~~~~");
        cursor.movePosition(QTextCursor::Left, QTextCursor::MoveAnchor, 2);
        cursor.endEditBlock();
        setTextCursor(cursor);
    }
}

void MarkdownEditor::insertLinkOrPicuture(int type, const QString &text,
                                          const QString &url,
                                          const QString &title,
                                          const QString &width,
                                          const QString &height){
    QTextCursor cursor = textCursor();
    cursor.beginEditBlock();
    cursor.clearSelection();

    if(conf->getMarkdownEngineType()==MarkdownToHtml::MultiMarkdown){
        QString insertText = QString::fromUtf8("[%1][]").arg(text);
        if(type==MdCharmGlobal::ShortcutInsertPicture)
            insertText.prepend("!");
        cursor.insertText(insertText);
        int curBlockNum = cursor.blockNumber();
        int cursorPosition = cursor.position();
        cursor.movePosition(QTextCursor::End);
        int endBlockNum = cursor.blockNumber();
        QString ref;
        if(endBlockNum==curBlockNum || (endBlockNum-1)==curBlockNum)
        {
            ref.append("\n\n");
        } else {
            QTextBlock endBlock = cursor.block();
            if(!endBlock.text().isEmpty() && endBlock.text()[0]=='['){
                ref.append("\n");
            } else {
                ref.append("\n\n");
            }
        }
        ref.append(QString::fromUtf8("[%1]: %2").arg(text).arg(url));
        if(!title.isEmpty())
            ref.append(" \""+title+"\"");
        if(!width.isEmpty())
            ref.append(" width="+width);
        if(!height.isEmpty())
            ref.append(" height="+height);
        cursor.insertText(ref);
        cursor.setPosition(cursorPosition);
    } else {
        QString insertText = QString::fromUtf8("[%1](%2 \"%3\")").arg(text).arg(url).arg(title);
        if(type==MdCharmGlobal::ShortcutInsertPicture)
            insertText.prepend("!");
        cursor.insertText(insertText);
    }

    cursor.endEditBlock();
    setTextCursor(cursor);
}

void MarkdownEditor::insertCode(const QString &lan)
{
    QTextCursor cursor = textCursor();
    cursor.beginEditBlock();
    cursor.clearSelection();
    if(!cursor.atBlockStart()){
        cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::MoveAnchor);
        cursor.insertBlock();
    }
    cursor.insertText("```"+lan+"\nInsert Your Code Here.\n```");
    cursor.movePosition(QTextCursor::PreviousBlock, QTextCursor::MoveAnchor);
    cursor.movePosition(QTextCursor::StartOfBlock, QTextCursor::MoveAnchor);
    cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
    cursor.endEditBlock();
    setTextCursor(cursor);
}

void MarkdownEditor::quoteText()
{
    QTextCursor cursor = textCursor();
    QString text("> ");
    if(cursor.hasSelection())
    {
        int start = cursor.selectionStart();
        QTextBlock block = document()->findBlock(cursor.selectionStart());
        QTextBlock endBlock = document()->findBlock(cursor.selectionEnd()).next();
        cursor.clearSelection();
        cursor.beginEditBlock();
        cursor.setPosition(start);
        cursor.movePosition(QTextCursor::StartOfBlock);
        do {
            cursor.insertText(text);
            cursor.movePosition(QTextCursor::NextBlock);
            block = block.next();
        } while(block.isValid() && block != endBlock);
        cursor.endEditBlock();
    } else {
        cursor.beginEditBlock();
        if(!cursor.atBlockStart())
            cursor.insertBlock();
        cursor.insertText(text);
        cursor.endEditBlock();
    }
}

void MarkdownEditor::untabText()
{
    QTextCursor cursor = textCursor();
    if (cursor.hasSelection()){
        int start = cursor.selectionStart();
        QTextBlock block = document()->findBlock(cursor.selectionStart());
        QTextBlock endBlock = document()->findBlock(cursor.selectionEnd()).next();
        cursor.clearSelection();
        cursor.beginEditBlock();
        cursor.setPosition(start);
        do {
            QString text = block.text();
            cursor.movePosition(QTextCursor::StartOfBlock);
            if(!text.isEmpty()&&!text.trimmed().isEmpty()){
                int i=0;
                while(i<text.size()&&i<4){//TODO: size
                    if(text[i]==QChar(' ')){
                        i++;
                        cursor.deleteChar();
                        continue;
                    }
                    if(text[i]==QChar('\t')){
                        if (i==0)
                            cursor.deleteChar();
                        break;
                    }
                    break;
                }
            }
            block = block.next();
            cursor.movePosition(QTextCursor::NextBlock);
        } while(block.isValid() && block != endBlock);
        cursor.endEditBlock();
    } else {
        QTextBlock block = document()->findBlock(cursor.position());
        QString text = block.text();
        int pos = cursor.positionInBlock();
        if(text.isEmpty()||text.trimmed().isEmpty())
            return;
        int i=0;
        for(i=0; i<pos && i<text.size(); i++){
            if(!text[i].isSpace() && !(text[i]=='\t'))
                break;
        }
        if(i<pos)
            return;
        //else
        if(i==0)
            i=text.size();
        cursor.beginEditBlock();
        cursor.movePosition(QTextCursor::StartOfBlock);
        for(int j=0; j<i&&j<4; j++){
            if(text[j].isSpace()){
                cursor.deleteChar();
                continue;
            }
            if(text[j]=='\t'){
                if(j==0)
                    cursor.deleteChar();
                break;
            }
        }
        cursor.endEditBlock();
    }
}

void MarkdownEditor::copyAsHtmlSlot()
{
    QTextCursor cursor = textCursor();
    if(!cursor.hasSelection())
        return;
    //Has selection
    QByteArray content = cursor.selectedText().replace(QChar::ParagraphSeparator, '\n').toUtf8();
    std::string textResult;
    MarkdownToHtml::translateMarkdownToHtml(conf->getMarkdownEngineType(), content.data(), content.length(), textResult);
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(QString::fromUtf8(textResult.c_str(), textResult.length()));
}

QList<QAction *> MarkdownEditor::addSpellCheckActions(QMenu *menu)
{
    QList<QAction *> spellCheckActions;
    if(!conf->isCheckSpell())
        return spellCheckActions;
    QTextCursor cursor = textCursor();
    cursor.select(QTextCursor::WordUnderCursor);
    QString text = cursor.selectedText();
    SpellChecker *sc = mdCharmGlobal->getSpellChecker(spellCheckLanguage);
    if(!sc)
        return spellCheckActions;
    if(sc->checkString(text).isEmpty())
        return spellCheckActions;
    QStringList sl = sc->suggest(text);
    if(sl.isEmpty())
        return spellCheckActions;
    foreach (QString s, sl) {
        spellCheckActions.append(new QAction(QIcon(Resource::SpellCheckIcon), s, menu));
    }
    QList<QAction *> actions = menu->actions();
    if(actions.isEmpty()){
        menu->addActions(spellCheckActions);
    } else {
        menu->insertActions(actions.first(), spellCheckActions);
        menu->insertSeparator(actions.first());
    }
    return spellCheckActions;
}

QList<QAction *> MarkdownEditor::addSpellCheckLanguageActions(QMenu *menu)
{
    if(spellCheckLanguage.isEmpty())
        return QList<QAction *>();
    QMenu *scl = new QMenu(tr("Languages"), menu);
    QActionGroup *lanGroup = new QActionGroup(scl);
    foreach (const QString s, mdCharmGlobal->getSpellCheckerLanguageList()) {
        QString dictLocaleName = mdCharmGlobal->getDictLocaleName(s);
        if(dictLocaleName.isEmpty())
            dictLocaleName = s;
        QAction *action = new QAction(dictLocaleName, scl);
        action->setCheckable(true);
        action->setData(s);
        scl->addAction(action);
        lanGroup->addAction(action);
        if(s==spellCheckLanguage)
            action->setChecked(true);
    }
    menu->addMenu(scl);
    return lanGroup->actions();
}
