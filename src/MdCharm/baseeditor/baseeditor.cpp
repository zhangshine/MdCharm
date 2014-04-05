#include <QtGui>
#include <QtCore>

#ifdef QT_V5
#include <QtPrintSupport>
#endif

#include "baseeditor.h"
#include "util/spellcheck/spellchecker.h"
#include "configuration.h"
#include "utils.h"

BaseEditor::BaseEditor(QWidget *parent) :
    QPlainTextEdit(parent)
{
    conf = Configuration::getInstance();
    mdCharmGlobal = MdCharmGlobal::getInstance();
    lineNumberArea = new LineNumberArea(this);

    displayLineNumber = false;
    finded = false;
    replacing = false;
    if(conf->isCheckSpell())
        spellCheckLanguage=conf->getSpellCheckLanguage();

    connect(this, SIGNAL(textChanged()),
            this, SLOT(ensureAtTheLast()));
}

BaseEditor::~BaseEditor(){}

void BaseEditor::initSpellCheckMatter()//triggered by setDocument()
{
    if(!conf->isCheckSpell())
        return;
    connect(document(), SIGNAL(contentsChange(int,int,int)),
            this, SLOT(spellCheck(int,int,int)));
    checkWholeContent();
}

void BaseEditor::enableSpellCheck()
{
    spellCheckErrorSelection.clear();
    disconnect(document(), SIGNAL(contentsChange(int,int,int)),
               this, SLOT(spellCheck(int,int,int)));
    updateExtraSelection();
    if(spellCheckLanguage.isEmpty())
        spellCheckLanguage=conf->getSpellCheckLanguage();
    if(mdCharmGlobal->getSpellChecker(spellCheckLanguage)==NULL){
        Q_ASSERT(0 && "This should not be happen");
        spellCheckLanguage.clear();
        return;
    }
    connect(document(), SIGNAL(contentsChange(int,int,int)),
            this, SLOT(spellCheck(int,int,int)));
    checkWholeContent();
}

void BaseEditor::disableSpellCheck()
{
    spellCheckErrorSelection.clear();
    spellCheckLanguage.clear();
    disconnect(document(), SIGNAL(contentsChange(int,int,int)),
               this, SLOT(spellCheck(int,int,int)));
    updateExtraSelection();;
}

void BaseEditor::setDocument(QTextDocument *doc)
{
    QPlainTextEdit::setDocument(doc);
    initSpellCheckMatter();
}

void BaseEditor::enableHighlightCurrentLine()
{
    highlightCurrentLine();
    connect(this, SIGNAL(cursorPositionChanged()),
            this, SLOT(highlightCurrentLine()));
}

void BaseEditor::disableHighlightCurrentLine()
{
    disconnect(this, SIGNAL(cursorPositionChanged()),
               this, SLOT(highlightCurrentLine()));
    currentLineSelection.clear();
    updateExtraSelection();
}

void BaseEditor::enableDisplayLineNumber()
{
    displayLineNumber = true;
    connect(this, SIGNAL(blockCountChanged(int)),
            this, SLOT(updateLineNumberAreaWidth(int)));
    connect(this, SIGNAL(updateRequest(QRect,int)),
            this, SLOT(updateLineNumberArea(QRect,int)));
    updateLineNumberAreaWidth(0);
    lineNumberArea->setVisible(true);
}

void BaseEditor::disableDisplayLineNumber()
{
    displayLineNumber = false;
    disconnect(this, SIGNAL(updateRequest(QRect,int)),
               this, SLOT(updateLineNumberArea(QRect,int)));
    disconnect(this, SIGNAL(blockCountChanged(int)),
               this, SLOT(updateLineNumberAreaWidth(int)));
    updateLineNumberAreaWidth(0);
    lineNumberArea->setVisible(false);
}

int BaseEditor::lineNumberAreaWidth()
{
    int digits = 1;
    int max = qMax(1, blockCount());
    while(max >= 10)
    {
        max /= 10;
        ++digits;
    }
    int space = 3 + QFontMetrics(document()->defaultFont()).width(QLatin1Char('9')) * digits;
    return space;
}

int BaseEditor::firstVisibleLineNumber()
{
    return firstVisibleBlock().blockNumber()+1;
}

void BaseEditor::findAndHighlightText(const QString &text, QTextDocument::FindFlags qff,
                                      bool isRE, bool isSetTextCursor)
{
    if(replacing)
        return;
    if(text.isEmpty())
    {
        findTextSelection.clear();
        currentFindSelection.clear();
        updateExtraSelection();
        return;
    }
    if(findAllOccurrance(text, qff, isRE))
    {
        finded = true;
        findFirstOccurrance(text, qff, isRE, true, isSetTextCursor);
    }
}
bool BaseEditor::findAllOccurrance(const QString &text, QTextDocument::FindFlags qff, bool isRE)
{
    QTextDocument *doc = document();

    findTextSelection.clear();
    bool finded=false;

    if(text.isEmpty())
    {
        prevFindCursor = QTextCursor();
        return finded;
    } else {
        QTextEdit::ExtraSelection es;
        QTextCursor highlightCursor(doc);

        QTextCharFormat plainFormat(highlightCursor.charFormat());
        QTextCharFormat colorFormat = plainFormat;
        colorFormat.setBackground(Qt::yellow);

        es.format = colorFormat;
        QRegExp re(text);

        while(!highlightCursor.isNull() && !highlightCursor.atEnd())
        {
            highlightCursor = isRE ? doc->find(re, highlightCursor, qff) :
                                     doc->find(text, highlightCursor, qff);

            if(!highlightCursor.isNull())
            {
                finded = true;
                es.cursor = highlightCursor;
                findTextSelection.append(es);
            }
        }
        if(!finded)
        {
            prevFindCursor = highlightCursor;
        }
        return finded;
    }
}

void BaseEditor::findFirstOccurrance(const QString &text, QTextDocument::FindFlags qff,
                                     bool isRE, bool init, bool isSetTextCusor)
{
    if (!finded)
        return;
    QRegExp re(text);
    QTextDocument *doc = document();
    QTextCursor currentCursor = textCursor();
    QTextCursor firstCursor;
    QTextEdit::ExtraSelection es;
    if(!init || prevFindCursor.isNull())
    {
        QTextCursor startCursor;
        if(qff&QTextDocument::FindBackward && !prevFindCursor.isNull())
        {
            prevFindCursor.movePosition(QTextCursor::Left, QTextCursor::MoveAnchor,
                      abs(prevFindCursor.selectionStart()-prevFindCursor.selectionEnd()));
        }
        if(prevFindCursor.isNull())
            startCursor = currentCursor;
        else
            startCursor = prevFindCursor;
        firstCursor = isRE ? doc->find(re, startCursor, qff):
                             doc->find(text, startCursor, qff);
    } else {
        firstCursor = isRE ? doc->find(re, prevFindCursor.selectionStart(), qff):
                             doc->find(text, prevFindCursor.selectionStart(), qff);
    }
    if(firstCursor.isNull())
    {
        QTextCursor wholeCursor(doc);
        if(qff & QTextDocument::FindBackward)
            wholeCursor.movePosition(QTextCursor::End);
        firstCursor = isRE ? doc->find(re, wholeCursor, qff):
                             doc->find(text, wholeCursor, qff);
    }
    if(firstCursor.isNull())
    {
        prevFindCursor = firstCursor;
        return;
    }
    es.cursor = firstCursor;
    QTextCharFormat f;
    f.setBackground(Qt::blue);
    f.setForeground(Qt::white);
    es.format = f;
    currentFindSelection.clear();
    currentFindSelection.append(es);
    prevFindCursor = firstCursor;
    firstCursor.clearSelection();
    if(isSetTextCusor)
        setTextCursor(firstCursor);
    ensureCursorVisible();
    updateExtraSelection();
}

void BaseEditor::updateLineNumberAreaWidth(int newBlockCount)
{
    Q_UNUSED(newBlockCount)
    setViewportMargins(displayLineNumber ? lineNumberAreaWidth() : 0, 0, 0, 0);
}

void BaseEditor::updateLineNumberArea(const QRect &rect, int dy)
{
    if (dy)
        lineNumberArea->scroll(0, dy);
    else
        lineNumberArea->update(0, rect.y(), lineNumberArea->width(), rect.height());

    if (rect.contains(viewport()->rect()))
        updateLineNumberAreaWidth(0);
}

void BaseEditor::resizeEvent(QResizeEvent *e)
{
    QPlainTextEdit::resizeEvent(e);
    if(!displayLineNumber)
        return;
    QRect cr = contentsRect();
    lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}

void BaseEditor::keyPressEvent(QKeyEvent *e)
{
    QPlainTextEdit::keyPressEvent(e);
    if(!e->isAccepted()&&e->key()==Qt::Key_Insert)//FIXME: crashrpt 8135582f-6f1c-46db-b3be-85ee2702a88d
    {
        emit overWriteModeChanged();
    }
}

void BaseEditor::focusInEvent(QFocusEvent *e)
{
    emit focusInSignal();
    QPlainTextEdit::focusInEvent(e);
}

static void fillBackground(QPainter *p, const QRectF &rect, QBrush brush, QRectF gradientRect = QRectF())//copy from QPlainTextEditor from 4.8.1
{
    p->save();
    if (brush.style() >= Qt::LinearGradientPattern && brush.style() <= Qt::ConicalGradientPattern) {
        if (!gradientRect.isNull()) {
            QTransform m = QTransform::fromTranslate(gradientRect.left(), gradientRect.top());
            m.scale(gradientRect.width(), gradientRect.height());
            brush.setTransform(m);
            const_cast<QGradient *>(brush.gradient())->setCoordinateMode(QGradient::LogicalMode);
        }
    } else {
        p->setBrushOrigin(rect.topLeft());
    }
    p->fillRect(rect, brush);
    p->restore();
}

static QColor blendColors(const QColor &a, const QColor &b, int alpha)//copy from QPlainTextEditor from 4.8.1
{
    return QColor((a.red()   * (256 - alpha) + b.red()   * alpha) / 256,
                  (a.green() * (256 - alpha) + b.green() * alpha) / 256,
                  (a.blue()  * (256 - alpha) + b.blue()  * alpha) / 256);
}

void BaseEditor::paintEvent(QPaintEvent *e)
{
    //copy from QPlainTextEditor
    QPainter painter(viewport());
    Q_ASSERT(qobject_cast<QPlainTextDocumentLayout*>(document()->documentLayout()));

    QPointF offset(contentOffset());

    QRect er = e->rect();
    QRect viewportRect = viewport()->rect();

    bool editable = !isReadOnly();

    QTextBlock block = firstVisibleBlock();
    qreal maximumWidth = document()->documentLayout()->documentSize().width();

    //margin
    qreal lineX = 0;
    if (conf->isDisplayRightColumnMargin()) {
        // Don't use QFontMetricsF::averageCharWidth here, due to it returning
        // a fractional size even when this is not supported by the platform.
        lineX = QFontMetricsF(document()->defaultFont()).width(QLatin1Char('X')) * conf->getRightMarginColumn() + offset.x() + 4;

        if (lineX < viewportRect.width()) {
            const QBrush background = QBrush(QColor(239, 239, 239));
            painter.fillRect(QRectF(lineX, er.top(), viewportRect.width() - lineX, er.height()),
                             background);

            const QColor col = (palette().base().color().value() > 128) ? Qt::black : Qt::white;
            const QPen pen = painter.pen();
            painter.setPen(blendColors(background.isOpaque() ? background.color() : palette().base().color(),
                                       col, 32));
            painter.drawLine(QPointF(lineX, er.top()), QPointF(lineX, er.bottom()));
            painter.setPen(pen);
        }
    }

    // Set a brush origin so that the WaveUnderline knows where the wave started
    painter.setBrushOrigin(offset);

    // keep right margin clean from full-width selection
    int maxX = offset.x() + qMax((qreal)viewportRect.width(), maximumWidth)
               - document()->documentMargin();
    er.setRight(qMin(er.right(), maxX));
    painter.setClipRect(er);


    QAbstractTextDocumentLayout::PaintContext context = getPaintContext();

    while (block.isValid()) {

        QRectF r = blockBoundingRect(block).translated(offset);
        QTextLayout *layout = block.layout();

        if (!block.isVisible()) {
            offset.ry() += r.height();
            block = block.next();
            continue;
        }

        if (r.bottom() >= er.top() && r.top() <= er.bottom()) {

            QTextBlockFormat blockFormat = block.blockFormat();

            QBrush bg = blockFormat.background();
            if (bg != Qt::NoBrush) {
                QRectF contentsRect = r;
                contentsRect.setWidth(qMax(r.width(), maximumWidth));
                fillBackground(&painter, contentsRect, bg);
            }


            QVector<QTextLayout::FormatRange> selections;
            int blpos = block.position();
            int bllen = block.length();
            for (int i = 0; i < context.selections.size(); ++i) {
                const QAbstractTextDocumentLayout::Selection &range = context.selections.at(i);
                const int selStart = range.cursor.selectionStart() - blpos;
                const int selEnd = range.cursor.selectionEnd() - blpos;
                if (selStart < bllen && selEnd > 0
                    && selEnd > selStart) {
                    QTextLayout::FormatRange o;
                    o.start = selStart;
                    o.length = selEnd - selStart;
                    o.format = range.format;
                    selections.append(o);
                } else if (!range.cursor.hasSelection() && range.format.hasProperty(QTextFormat::FullWidthSelection)
                           && block.contains(range.cursor.position())) {
                    // for full width selections we don't require an actual selection, just
                    // a position to specify the line. that's more convenience in usage.
                    QTextLayout::FormatRange o;
                    QTextLine l = layout->lineForTextPosition(range.cursor.position() - blpos);
                    o.start = l.textStart();
                    o.length = l.textLength();
                    if (o.start + o.length == bllen - 1)
                        ++o.length; // include newline
                    o.format = range.format;
                    selections.append(o);
                }
            }

            bool drawCursor = ((editable || (textInteractionFlags() & Qt::TextSelectableByKeyboard))
                               && context.cursorPosition >= blpos
                               && context.cursorPosition < blpos + bllen);

            bool drawCursorAsBlock = drawCursor && overwriteMode() ;

            if (drawCursorAsBlock) {
                if (context.cursorPosition == blpos + bllen - 1) {
                    drawCursorAsBlock = false;
                } else {
                    QTextLayout::FormatRange o;
                    o.start = context.cursorPosition - blpos;
                    o.length = 1;
                    o.format.setForeground(palette().base());
                    o.format.setBackground(palette().text());
                    selections.append(o);
                }
            }


            layout->draw(&painter, offset, selections, er);
            if ((drawCursor && !drawCursorAsBlock)
                || (editable && context.cursorPosition < -1
                    && !layout->preeditAreaText().isEmpty())) {
                int cpos = context.cursorPosition;
                if (cpos < -1)
                    cpos = layout->preeditAreaPosition() - (cpos + 2);
                else
                    cpos -= blpos;
                layout->drawCursor(&painter, offset, cpos, cursorWidth());
            }
        }

        offset.ry() += r.height();
        if (offset.y() > viewportRect.height())
            break;
        block = block.next();
    }

    if (backgroundVisible() && !block.isValid() && offset.y() <= er.bottom()
        && (centerOnScroll() || verticalScrollBar()->maximum() == verticalScrollBar()->minimum())) {
        painter.fillRect(QRect(QPoint((int)er.left(), (int)offset.y()), er.bottomRight()), palette().background());
    }
}

void BaseEditor::highlightCurrentLine()
{
    currentLineSelection.clear();
    if (!isReadOnly())
    {
        QTextEdit::ExtraSelection selection;
        QColor lineColor = QColor::fromRgb(0xC6, 0xE2, 0xFF);

        selection.format.setBackground(lineColor);
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        selection.cursor = textCursor();
        selection.cursor.clearSelection();;
        currentLineSelection.append(selection);
    }
    updateExtraSelection();
}

void BaseEditor::lineNumberAreaPaintEvent(QPaintEvent *event)
{
    QPainter painter(lineNumberArea);
    painter.fillRect(event->rect(), QColor::fromRgb(0xEA,0xEA,0xEA));
    painter.setFont(document()->defaultFont());

    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = (int) blockBoundingGeometry(block).translated(contentOffset()).top();
    int bottom = top + (int) blockBoundingRect(block).height();
    int height = QFontMetrics(document()->defaultFont()).height();
    while(block.isValid() && top <= event->rect().bottom())
    {
        if (block.isVisible() && bottom >= event->rect().top())
        {
            QString number = QString::number(blockNumber+1);
            painter.setPen(Qt::black);
            painter.drawText(0, top, lineNumberArea->width(), height,
                             Qt::AlignRight, number);
        }
        block = block.next();
        top = bottom;
        bottom = top + (int) blockBoundingRect(block).height();
        ++blockNumber;
    }
}

void BaseEditor::updateExtraSelection()
{
    setExtraSelections(findTextSelection+currentLineSelection+currentFindSelection+spellCheckErrorSelection);
}

void BaseEditor::findFinished()
{
    findTextSelection.clear();
    currentFindSelection.clear();
    updateExtraSelection();
    prevFindCursor = QTextCursor();
    finded = false;
}

void BaseEditor::replace(const QString &rt)
{
    if(prevFindCursor.isNull())
        return;
    prevFindCursor.beginEditBlock();
    prevFindCursor.insertText(rt);
    prevFindCursor.endEditBlock();
}

void BaseEditor::replaceAll(const QString &ft, const QString &rt,
                            QTextDocument::FindFlags qff, bool isRE)
{
    QTextDocument *doc = document();
    QTextCursor tc(doc);
    QRegExp re(ft);
    replacing = true;
    while(!tc.isNull() && !tc.atEnd())
    {
        tc = isRE ? doc->find(re, tc, qff) :
                                 doc->find(ft, tc, qff);
        if(!tc.isNull())
        {
            tc.beginEditBlock();
            tc.insertText(rt);
            tc.endEditBlock();
        }
    }
    replacing = false;
    findAndHighlightText(ft, qff, isRE);
}

void BaseEditor::ensureAtTheLast()
{
    QTextCursor tc = textCursor();
    if(tc.atEnd())
        verticalScrollBar()->setValue(verticalScrollBar()->maximum());
}

void BaseEditor::spellCheck(int start, int unused, int length)
{
    Q_UNUSED(unused)
    if(length==0)
        return;
//    qDebug("start %d, length %d", start, length);;
    int end = start+length;
    bool isInSameBlock=false;
    if(start==end)
        isInSameBlock = true;
    QTextBlock startBlock = document()->findBlock(start);
    QTextBlock endBlock = document()->findBlock(end);
    if(!endBlock.isValid())
        endBlock = document()->lastBlock();
//    qDebug("start block %d, end block %d", startBlock.blockNumber(), endBlock.blockNumber());
    if(startBlock.blockNumber()==endBlock.blockNumber())
        isInSameBlock = true;
    spellCheckAux(startBlock);
    if(!isInSameBlock){
        for(int i=0; i<endBlock.blockNumber()-startBlock.blockNumber(); i++){
            spellCheckAux(document()->findBlockByNumber(startBlock.blockNumber()+i+1));
        }
    }
    updateExtraSelection();
}

void BaseEditor::spellCheckAux(const QTextBlock &block)
{
    removeExtraSelectionInRange(spellCheckErrorSelection, block.position(), block.position()+block.length());
    SpellChecker *spellChecker = mdCharmGlobal->getSpellChecker(spellCheckLanguage);
    if(spellChecker==NULL)
        return;
    SpellCheckResultList resultList = spellChecker->checkString(block.text());
    QTextCharFormat spellErrorCharFormat;
    spellErrorCharFormat.setUnderlineStyle(QTextCharFormat::WaveUnderline);
    spellErrorCharFormat.setUnderlineColor(Qt::darkRed);
    for(int i=0; i<resultList.length(); i++){
        SpellCheckResult result = resultList.at(i);
        QTextCursor errorCursor(block);
        errorCursor.setPosition(block.position()+result.start);
        errorCursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, result.end-result.start);//select wrong words
        QTextEdit::ExtraSelection es;
        es.cursor = errorCursor;
        es.format = spellErrorCharFormat;
        spellCheckErrorSelection.append(es);
    }
}

void BaseEditor::checkWholeContent()
{
    for(int i=0; i<blockCount(); i++)
        spellCheckAux(document()->findBlockByNumber(i));
    updateExtraSelection();
}

void BaseEditor::removeExtraSelectionInRange(QList<QTextEdit::ExtraSelection> &extraList, int start, int end)
{
    QStack<int> toRemove;
    for(int i=0; i<extraList.length(); i++)
    {
        QTextEdit::ExtraSelection es = extraList.at(i);
        QTextCursor tc= es.cursor;
        if(tc.selectionStart()>=start && tc.selectionEnd()<=end)
            toRemove.push(i);
    }
    //remove from end to begin
    while(!toRemove.isEmpty())
        extraList.removeAt(toRemove.pop());
}
