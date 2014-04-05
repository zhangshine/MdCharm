#include <QtGui>
#include <QtCore>

#include <cassert>

#include "odtwriter.h"
#include "configuration.h"

static QString pixelToPoint(qreal pixels)
{
    return QString::number(pixels * 72/96) + QString::fromLatin1("pt");
}

static QString bulletChar(QTextListFormat::Style style)
{
    switch(style) {
    case QTextListFormat::ListDisc:
        return QChar(0x25cf); // bullet character
    case QTextListFormat::ListCircle:
        return QChar(0x25cb); // white circle
    case QTextListFormat::ListSquare:
        return QChar(0x25a1); // white square
    case QTextListFormat::ListDecimal:
        return QString::fromLatin1("1");
    case QTextListFormat::ListLowerAlpha:
        return QString::fromLatin1("a");
    case QTextListFormat::ListUpperAlpha:
        return QString::fromLatin1("A");
    case QTextListFormat::ListLowerRoman:
        return QString::fromLatin1("i");
    case QTextListFormat::ListUpperRoman:
        return QString::fromLatin1("I");
    default:
    case QTextListFormat::ListStyleUndefined:
        return QString();
    }
}

ODTWriter::ODTWriter(const QTextDocument &document, const QString &fileName):
    document(document), manifestWriter(&manifest), contentWriter(&content),
    metaWriter(&meta), stylesWriter(&styles)
{
    zip = new ZipWriter(fileName);
    conf = Configuration::getInstance();

    officeNS =  QLatin1String("urn:oasis:names:tc:opendocument:xmlns:office:1.0");
    metaNS =    QLatin1String("urn:oasis:names:tc:opendocument:xmlns:meta:1.0");
    configNS =  QLatin1String("urn:oasis:names:tc:opendocument:xmlns:config:1.0");
    textNS =    QLatin1String("urn:oasis:names:tc:opendocument:xmlns:text:1.0");
    tableNS =   QLatin1String("urn:oasis:names:tc:opendocument:xmlns:table:1.0");
    drawNS =    QLatin1String("urn:oasis:names:tc:opendocument:xmlns:drawing:1.0");
    presentationNS = QLatin1String("urn:oasis:names:tc:opendocument:xmlns:presentation:1.0");
    chartNS =   QLatin1String("urn:oasis:names:tc:opendocument:xmlns:chart:1.0");
    formNS =    QLatin1String("urn:oasis:names:tc:opendocument:xmlns:form:1.0");
    scriptNS =  QLatin1String("urn:oasis:names:tc:opendocument:xmlns:script:1.0");
    styleNS =   QLatin1String("urn:oasis:names:tc:opendocument:xmlns:style:1.0");
    numberNS =  QLatin1String("urn:oasis:names:tc:opendocument:xmlns:data sytle:1.0");
    animNS =    QLatin1String("urn:oasis:names:tc:opendocument:xmlns:animation:1.0");
    dcNS =      QLatin1String("http://purl.org/dc/elements/1.1/");
    xlinkNS =   QLatin1String("http://www.w3.org/1999/xlink");
    mathNS =    QLatin1String("http://www.w3.org/1998/Math/MathML");
    xformNS =   QLatin1String("http://www.w3.org/2002/xforms");
    foNS =      QLatin1String("urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0");
    svgNS =     QLatin1String("urn:oasis:names:tc:opendocument:xmlns:svg-compatible:1.0");
    smilNS =    QLatin1String("urn:oasis:names:tc:opendocument:xmlns:smil-compatible:1.0");


    manifestNS= QLatin1String("urn:oasis:names:tc:opendocument:xmlns:manifest:1.0");

    picCounter = 0;
}

ODTWriter::~ODTWriter()
{
    delete zip;
}

bool ODTWriter::writeAll()
{
    assert(zip->isWritable());

    manifestWriter.setAutoFormatting(true);
    manifestWriter.setAutoFormattingIndent(1);

    manifestWriter.writeNamespace(manifestNS, QString::fromLatin1("manifest"));
    manifestWriter.writeStartDocument();
    manifestWriter.writeStartElement(manifestNS, QString::fromLatin1("manifest"));
    //manifestWriter.writeAttribute(manifestNS, QString::fromLatin1("version"), QString::fromLatin1("1.2"));
    addFile(QString::fromLatin1("/"), QString::fromLatin1("application/vnd.oasis.opendocument.text"));

    contentWriter.setAutoFormatting(true);
    contentWriter.setAutoFormattingIndent(2);

    writeNameSpaces(contentWriter);
    contentWriter.writeStartDocument();
    contentWriter.writeStartElement(officeNS, QString::fromLatin1("document-content"));
    contentWriter.writeAttribute(officeNS, QString::fromLatin1("version"), QString::fromLatin1("1.2"));

    // add fragments. (for character formats)
//    QTextDocumentPrivate::FragmentIterator fragIt = document.docHandle()->begin();
//    QSet<int> formats;
//    while (fragIt != m_document->docHandle()->end()) {
//        const QTextFragmentData * const frag = fragIt.value();
//        formats << frag->format;
//        ++fragIt;
//    }

//    // add blocks (for blockFormats)
//    QTextDocumentPrivate::BlockMap &blocks = m_document->docHandle()->blockMap();
//    QTextDocumentPrivate::BlockMap::Iterator blockIt = blocks.begin();
//    while (blockIt != blocks.end()) {
//        const QTextBlockData * const block = blockIt.value();
//        formats << block->format;
//        ++blockIt;
//    }

    // add objects for lists, frames and tables
//    QVector<QTextFormat> allFormats = document.allFormats();
//    QList<int> copy = formats.toList();
//    for (QList<int>::Iterator iter = copy.begin(); iter != copy.end(); ++iter) {
//        QTextObject *object = document.objectForFormat(allFormats[*iter]);
//        if (object)
//            formats << object->formatIndex();
//    }

    QSet<int> formats;

    //int blockCount = document.blockCount();
    for (QTextBlock it = document.begin(); it != document.end(); it = it.next())
    {
        formats << it.blockFormatIndex();
        for(QTextBlock::iterator ita = it.begin(); !(ita.atEnd()); ++ita)
        {
            QTextFragment currentFragment = ita.fragment();
            if(currentFragment.isValid())
            {
                formats << currentFragment.charFormatIndex();
            }
        }
    }

    writeFormats(contentWriter, formats);

    contentWriter.writeStartElement(officeNS, QString::fromLatin1("body"));
    contentWriter.writeStartElement(officeNS, QString::fromLatin1("text"));
    QTextFrame *rootFrame = document.rootFrame();
    writeFrame(contentWriter, rootFrame);
    contentWriter.writeEndElement(); // text
    contentWriter.writeEndElement(); // body
    contentWriter.writeEndElement(); // document-content
    contentWriter.writeEndDocument();

    metaWriter.setAutoFormatting(true);
    metaWriter.setAutoFormattingIndent(1);
    writeNameSpaces(metaWriter);
    metaWriter.writeStartDocument();
    metaWriter.writeStartElement(officeNS, QString::fromLatin1("document-meta"));
    metaWriter.writeStartElement(officeNS, QString::fromLatin1("meta"));
    metaWriter.writeEndElement();
    metaWriter.writeEndElement();
    metaWriter.writeEndDocument();

    stylesWriter.setAutoFormatting(true);
    stylesWriter.setAutoFormattingIndent(1);
    writeNameSpaces(stylesWriter);
    stylesWriter.writeStartDocument();
    stylesWriter.writeStartElement(officeNS, QString::fromLatin1("document-styles"));
    stylesWriter.writeStartElement(officeNS, QString::fromLatin1("styles"));
    stylesWriter.writeEndElement();
    stylesWriter.writeEndElement();
    stylesWriter.writeEndDocument();

    writeMimeType();

    writeStyles();
    writeMeta();

    writeContent();

    manifestWriter.writeEndDocument();
    writeManifest();

    zip->close();
    return true;
}

void ODTWriter::writeBlock(QXmlStreamWriter &writer, const QTextBlock &block)
{
    if (block.textList()) { // its a list-item
          const int listLevel = block.textList()->format().indent();
          if (m_listStack.isEmpty() || m_listStack.top() != block.textList()) {
              // not the same list we were in.
              while (m_listStack.count() >= listLevel && !m_listStack.isEmpty() && m_listStack.top() != block.textList() ) { // we need to close tags
                  m_listStack.pop();
                  writer.writeEndElement(); // list
                  if (m_listStack.count())
                      writer.writeEndElement(); // list-item
              }
              while (m_listStack.count() < listLevel) {
                  if (m_listStack.count())
                      writer.writeStartElement(textNS, QString::fromLatin1("list-item"));
                  writer.writeStartElement(textNS, QString::fromLatin1("list"));
                  if (m_listStack.count() == listLevel - 1) {
                      m_listStack.push(block.textList());
                      writer.writeAttribute(textNS, QString::fromLatin1("style-name"), QString::fromLatin1("L%1")
                              .arg(block.textList()->formatIndex()));
                  }
                  else {
                      m_listStack.push(0);
                  }
              }
          }
          writer.writeStartElement(textNS, QString::fromLatin1("list-item"));
      }
      else {
          while (! m_listStack.isEmpty()) {
              m_listStack.pop();
              writer.writeEndElement(); // list
              if (m_listStack.count())
                  writer.writeEndElement(); // list-item
          }
      }

      if (block.length() == 1) { // only a linefeed
          writer.writeEmptyElement(textNS, QString::fromLatin1("p"));
          writer.writeAttribute(textNS, QString::fromLatin1("style-name"), QString::fromLatin1("p%1")
              .arg(block.blockFormatIndex()));
          if (block.textList())
              writer.writeEndElement(); // numbered-paragraph
          return;
      }
      writer.writeStartElement(textNS, QString::fromLatin1("p"));
      writer.writeAttribute(textNS, QString::fromLatin1("style-name"), QString::fromLatin1("p%1")
          .arg(block.blockFormatIndex()));
      for (QTextBlock::Iterator frag= block.begin(); !frag.atEnd(); frag++) {
          writer.writeCharacters(QString()); // Trick to make sure that the span gets no linefeed in front of it.
          writer.writeStartElement(textNS, QString::fromLatin1("span"));

          QString fragmentText = frag.fragment().text();
          if (fragmentText.length() == 1 && fragmentText[0] == 0xFFFC) { // its an inline character.
              writeInlineCharacter(writer, frag.fragment());
              writer.writeEndElement(); // span
              continue;
          }

          writer.writeAttribute(textNS, QString::fromLatin1("style-name"), QString::fromLatin1("c%1")
              .arg(frag.fragment().charFormatIndex()));
          bool escapeNextSpace = true;
          int precedingSpaces = 0;
          int exportedIndex = 0;
          for (int i=0; i <= fragmentText.count(); ++i) {
              bool isSpace = false;
                  QChar character = fragmentText[i];
                  isSpace = character.unicode() == ' ';

              // find more than one space. -> <text:s text:c="2" />
              if (!isSpace && escapeNextSpace && precedingSpaces > 1) {
                  const bool startParag = exportedIndex == 0 && i == precedingSpaces;
                  if (!startParag)
                      writer.writeCharacters(fragmentText.mid(exportedIndex, i - precedingSpaces + 1 - exportedIndex));
                  writer.writeEmptyElement(textNS, QString::fromLatin1("s"));
                  const int count = precedingSpaces - (startParag?0:1);
                  if (count > 1)
                      writer.writeAttribute(textNS, QString::fromLatin1("c"), QString::number(count));
                  precedingSpaces = 0;
                  exportedIndex = i;
              }

              if (i < fragmentText.count()) {
                  if (character.unicode() == 0x2028) { // soft-return
                      //if (exportedIndex < i)
                          writer.writeCharacters(fragmentText.mid(exportedIndex, i - exportedIndex));
                      writer.writeEmptyElement(textNS, QString::fromLatin1("line-break"));
                      exportedIndex = i+1;
                      continue;
                  } else if (character.unicode() == '\t') { // Tab
                      //if (exportedIndex < i)
                          writer.writeCharacters(fragmentText.mid(exportedIndex, i - exportedIndex));
                      writer.writeEmptyElement(textNS, QString::fromLatin1("tab"));
                      exportedIndex = i+1;
                      precedingSpaces = 0;
                  } else if (isSpace) {
                      ++precedingSpaces;
                      escapeNextSpace = true;
                  } else if (!isSpace) {
                      precedingSpaces = 0;
                  }
              }
          }

          writer.writeCharacters(fragmentText.mid(exportedIndex));
          writer.writeEndElement(); // span
      }
      writer.writeCharacters(QString()); // Trick to make sure that the span gets no linefeed behind it.
      writer.writeEndElement(); // p
      if (block.textList())
          writer.writeEndElement(); // list-item
}

void ODTWriter::writeFormats(QXmlStreamWriter &writer, QSet<int> formats) const
{
    writer.writeStartElement(officeNS, QString::fromLatin1("automatic-styles"));
    QVector<QTextFormat> allStyles = document.allFormats();
    QSetIterator<int> formatId(formats);
    while(formatId.hasNext()) {
        int formatIndex = formatId.next();
        QTextFormat textFormat = allStyles.at(formatIndex);
        switch (textFormat.type()) {
        case QTextFormat::CharFormat:
            if (textFormat.isTableCellFormat())
                writeTableCellFormat(writer, textFormat.toTableCellFormat(), formatIndex);
            else
                writeCharacterFormat(writer, textFormat.toCharFormat(), formatIndex);
            break;
        case QTextFormat::BlockFormat:
            writeBlockFormat(writer, textFormat.toBlockFormat(), formatIndex);
            break;
        case QTextFormat::ListFormat:
            writeListFormat(writer, textFormat.toListFormat(), formatIndex);
            break;
        case QTextFormat::FrameFormat:
            writeFrameFormat(writer, textFormat.toFrameFormat(), formatIndex);
            break;
        case QTextFormat::TableFormat:
            break;
        default:
            break;
        }
    }

    writer.writeEndElement(); // automatic-styles
}

void ODTWriter::writeBlockFormat(QXmlStreamWriter &writer, QTextBlockFormat format, int formatIndex) const
{
    writer.writeStartElement(styleNS, QString::fromLatin1("style"));
    writer.writeAttribute(styleNS, QString::fromLatin1("name"), QString::fromLatin1("p%1").arg(formatIndex));
    writer.writeAttribute(styleNS, QString::fromLatin1("family"), QString::fromLatin1("paragraph"));
    writer.writeStartElement(styleNS, QString::fromLatin1("paragraph-properties"));

    if (format.hasProperty(QTextFormat::BlockAlignment)) {
        const Qt::Alignment alignment = format.alignment() & Qt::AlignHorizontal_Mask;
        QString value;
        if (alignment == Qt::AlignLeading)
            value = QString::fromLatin1("start");
        else if (alignment == Qt::AlignTrailing)
            value = QString::fromLatin1("end");
        else if (alignment == (Qt::AlignLeft | Qt::AlignAbsolute))
            value = QString::fromLatin1("left");
        else if (alignment == (Qt::AlignRight | Qt::AlignAbsolute))
            value = QString::fromLatin1("right");
        else if (alignment == Qt::AlignHCenter)
            value = QString::fromLatin1("center");
        else if (alignment == Qt::AlignJustify)
            value = QString::fromLatin1("justify");
        else
            qWarning() << "QTextOdfWriter: unsupported paragraph alignment; " << format.alignment();
        if (! value.isNull())
            writer.writeAttribute(foNS, QString::fromLatin1("text-align"), value);
    }

    if (format.hasProperty(QTextFormat::BlockTopMargin))
        writer.writeAttribute(foNS, QString::fromLatin1("margin-top"), pixelToPoint(qMax(qreal(0.), format.topMargin())) );
    if (format.hasProperty(QTextFormat::BlockBottomMargin))
        writer.writeAttribute(foNS, QString::fromLatin1("margin-bottom"), pixelToPoint(qMax(qreal(0.), format.bottomMargin())) );
    if (format.hasProperty(QTextFormat::BlockLeftMargin) || format.hasProperty(QTextFormat::BlockIndent))
        writer.writeAttribute(foNS, QString::fromLatin1("margin-left"), pixelToPoint(qMax(qreal(0.),
            format.leftMargin() + format.indent())));
    if (format.hasProperty(QTextFormat::BlockRightMargin))
        writer.writeAttribute(foNS, QString::fromLatin1("margin-right"), pixelToPoint(qMax(qreal(0.), format.rightMargin())) );
    if (format.hasProperty(QTextFormat::TextIndent))
        writer.writeAttribute(foNS, QString::fromLatin1("text-indent"), pixelToPoint(format.textIndent()));
    if (format.hasProperty(QTextFormat::PageBreakPolicy)) {
        if (format.pageBreakPolicy() & QTextFormat::PageBreak_AlwaysBefore)
            writer.writeAttribute(foNS, QString::fromLatin1("break-before"), QString::fromLatin1("page"));
        if (format.pageBreakPolicy() & QTextFormat::PageBreak_AlwaysAfter)
            writer.writeAttribute(foNS, QString::fromLatin1("break-after"), QString::fromLatin1("page"));
    }
    if (format.hasProperty(QTextFormat::BackgroundBrush)) {
        QBrush brush = format.background();
        writer.writeAttribute(foNS, QString::fromLatin1("background-color"), brush.color().name());
    }
    if (format.hasProperty(QTextFormat::BlockNonBreakableLines))
        writer.writeAttribute(foNS, QString::fromLatin1("keep-together"),
                format.nonBreakableLines() ? QString::fromLatin1("true") : QString::fromLatin1("false"));
    if (format.hasProperty(QTextFormat::TabPositions)) {
        QList<QTextOption::Tab> tabs = format.tabPositions();
        writer.writeStartElement(styleNS, QString::fromLatin1("tab-stops"));
        QList<QTextOption::Tab>::Iterator iterator = tabs.begin();
        while(iterator != tabs.end()) {
            writer.writeEmptyElement(styleNS, QString::fromLatin1("tab-stop"));
            writer.writeAttribute(styleNS, QString::fromLatin1("position"), pixelToPoint(iterator->position) );
            QString type;
            switch(iterator->type) {
            case QTextOption::DelimiterTab: type = QString::fromLatin1("char"); break;
            case QTextOption::LeftTab: type = QString::fromLatin1("left"); break;
            case QTextOption::RightTab: type = QString::fromLatin1("right"); break;
            case QTextOption::CenterTab: type = QString::fromLatin1("center"); break;
            }
            writer.writeAttribute(styleNS, QString::fromLatin1("type"), type);
            if (iterator->delimiter != 0)
                writer.writeAttribute(styleNS, QString::fromLatin1("char"), iterator->delimiter);
            ++iterator;
        }

        writer.writeEndElement(); // tab-stops
    }

    writer.writeEndElement(); // paragraph-properties
    writer.writeEndElement(); // style
}

void ODTWriter::writeCharacterFormat(QXmlStreamWriter &writer, QTextCharFormat format, int formatIndex) const
{
    writer.writeStartElement(styleNS, QString::fromLatin1("style"));
        writer.writeAttribute(styleNS, QString::fromLatin1("name"), QString::fromLatin1("c%1").arg(formatIndex));
        writer.writeAttribute(styleNS, QString::fromLatin1("family"), QString::fromLatin1("text"));
        writer.writeEmptyElement(styleNS, QString::fromLatin1("text-properties"));
        if (format.fontItalic())
            writer.writeAttribute(foNS, QString::fromLatin1("font-style"), QString::fromLatin1("italic"));
        if (format.hasProperty(QTextFormat::FontWeight) && format.fontWeight() != QFont::Normal) {
            QString value;
            if (format.fontWeight() == QFont::Bold)
                value = QString::fromLatin1("bold");
            else
                value = QString::number(format.fontWeight() * 10);
            writer.writeAttribute(foNS, QString::fromLatin1("font-weight"), value);
        }
        if (format.hasProperty(QTextFormat::FontFamily))
            writer.writeAttribute(foNS, QString::fromLatin1("font-family"), format.fontFamily());
        else
            writer.writeAttribute(foNS, QString::fromLatin1("font-family"), conf->getFontFamily()); // default font
        if (format.hasProperty(QTextFormat::FontPointSize))
            writer.writeAttribute(foNS, QString::fromLatin1("font-size"), QString::fromLatin1("%1pt").arg(format.fontPointSize()));
        if (format.hasProperty(QTextFormat::FontCapitalization)) {
            switch(format.fontCapitalization()) {
            case QFont::MixedCase:
                writer.writeAttribute(foNS, QString::fromLatin1("text-transform"), QString::fromLatin1("none")); break;
            case QFont::AllUppercase:
                writer.writeAttribute(foNS, QString::fromLatin1("text-transform"), QString::fromLatin1("uppercase")); break;
            case QFont::AllLowercase:
                writer.writeAttribute(foNS, QString::fromLatin1("text-transform"), QString::fromLatin1("lowercase")); break;
            case QFont::Capitalize:
                writer.writeAttribute(foNS, QString::fromLatin1("text-transform"), QString::fromLatin1("capitalize")); break;
            case QFont::SmallCaps:
                writer.writeAttribute(foNS, QString::fromLatin1("font-variant"), QString::fromLatin1("small-caps")); break;
            }
        }
        if (format.hasProperty(QTextFormat::FontLetterSpacing))
            writer.writeAttribute(foNS, QString::fromLatin1("letter-spacing"), pixelToPoint(format.fontLetterSpacing()));
        if (format.hasProperty(QTextFormat::FontWordSpacing) && format.fontWordSpacing() != 0)
                writer.writeAttribute(foNS, QString::fromLatin1("word-spacing"), pixelToPoint(format.fontWordSpacing()));
        if (format.hasProperty(QTextFormat::FontUnderline))
            writer.writeAttribute(styleNS, QString::fromLatin1("text-underline-type"),
                    format.fontUnderline() ? QString::fromLatin1("single") : QString::fromLatin1("none"));
        if (format.hasProperty(QTextFormat::FontOverline)) {
            //   bool   fontOverline () const  TODO
        }
        if (format.hasProperty(QTextFormat::FontStrikeOut))
            writer.writeAttribute(styleNS,QString::fromLatin1( "text-line-through-type"),
                    format.fontStrikeOut() ? QString::fromLatin1("single") : QString::fromLatin1("none"));
        if (format.hasProperty(QTextFormat::TextUnderlineColor))
            writer.writeAttribute(styleNS, QString::fromLatin1("text-underline-color"), format.underlineColor().name());
        if (format.hasProperty(QTextFormat::FontFixedPitch)) {
            //   bool   fontFixedPitch () const  TODO
        }
        if (format.hasProperty(QTextFormat::TextUnderlineStyle)) {
            QString value;
            switch (format.underlineStyle()) {
            case QTextCharFormat::NoUnderline: value = QString::fromLatin1("none"); break;
            case QTextCharFormat::SingleUnderline: value = QString::fromLatin1("solid"); break;
            case QTextCharFormat::DashUnderline: value = QString::fromLatin1("dash"); break;
            case QTextCharFormat::DotLine: value = QString::fromLatin1("dotted"); break;
            case QTextCharFormat::DashDotLine: value = QString::fromLatin1("dash-dot"); break;
            case QTextCharFormat::DashDotDotLine: value = QString::fromLatin1("dot-dot-dash"); break;
            case QTextCharFormat::WaveUnderline: value = QString::fromLatin1("wave"); break;
            case QTextCharFormat::SpellCheckUnderline: value = QString::fromLatin1("none"); break;
            }
            writer.writeAttribute(styleNS, QString::fromLatin1("text-underline-style"), value);
        }
        if (format.hasProperty(QTextFormat::TextVerticalAlignment)) {
            QString value;
            switch (format.verticalAlignment()) {
            case QTextCharFormat::AlignMiddle:
            case QTextCharFormat::AlignBaseline:
            case QTextCharFormat::AlignNormal: value = QString::fromLatin1("0%"); break;
            case QTextCharFormat::AlignSuperScript: value = QString::fromLatin1("super"); break;
            case QTextCharFormat::AlignSubScript: value = QString::fromLatin1("sub"); break;
            case QTextCharFormat::AlignTop: value = QString::fromLatin1("100%"); break;
            case QTextCharFormat::AlignBottom : value = QString::fromLatin1("-100%"); break;
            }
            writer.writeAttribute(styleNS, QString::fromLatin1("text-position"), value);
        }
        if (format.hasProperty(QTextFormat::TextOutline))
            writer.writeAttribute(styleNS, QString::fromLatin1("text-outline"), QString::fromLatin1("true"));
        if (format.hasProperty(QTextFormat::TextToolTip)) {
            //   QString   toolTip () const  TODO
        }
        if (format.hasProperty(QTextFormat::IsAnchor)) {
            //   bool   isAnchor () const  TODO
        }
        if (format.hasProperty(QTextFormat::AnchorHref)) {
            //   QString   anchorHref () const  TODO
        }
        if (format.hasProperty(QTextFormat::AnchorName)) {
            //   QString   anchorName () const  TODO
        }
        if (format.hasProperty(QTextFormat::ForegroundBrush)) {
            QBrush brush = format.foreground();
            writer.writeAttribute(foNS, QString::fromLatin1("color"), brush.color().name());
        }
        if (format.hasProperty(QTextFormat::BackgroundBrush)) {
            QBrush brush = format.background();
            writer.writeAttribute(foNS, QString::fromLatin1("background-color"), brush.color().name());
        }

        writer.writeEndElement();
}

void ODTWriter::writeListFormat(QXmlStreamWriter &writer, QTextListFormat format, int formatIndex) const
{
    writer.writeStartElement(textNS, QString::fromLatin1("list-style"));
    writer.writeAttribute(styleNS, QString::fromLatin1("name"), QString::fromLatin1("L%1").arg(formatIndex));

    QTextListFormat::Style style = format.style();
    if (style == QTextListFormat::ListDecimal || style == QTextListFormat::ListLowerAlpha
            || style == QTextListFormat::ListUpperAlpha
            || style == QTextListFormat::ListLowerRoman
            || style == QTextListFormat::ListUpperRoman) {
        writer.writeStartElement(textNS, QString::fromLatin1("list-level-style-number"));
        writer.writeAttribute(styleNS, QString::fromLatin1("num-format"), bulletChar(style));

        if (format.hasProperty(QTextFormat::ListNumberSuffix))
            writer.writeAttribute(styleNS, QString::fromLatin1("num-suffix"), format.numberSuffix());
        else
            writer.writeAttribute(styleNS, QString::fromLatin1("num-suffix"), QString::fromLatin1("."));

        if (format.hasProperty(QTextFormat::ListNumberPrefix))
            writer.writeAttribute(styleNS, QString::fromLatin1("num-prefix"), format.numberPrefix());

    } else {
        writer.writeStartElement(textNS, QString::fromLatin1("list-level-style-bullet"));
        writer.writeAttribute(textNS, QString::fromLatin1("bullet-char"), bulletChar(style));
    }

    writer.writeAttribute(textNS, QString::fromLatin1("level"), QString::number(format.indent()));
    writer.writeEmptyElement(styleNS, QString::fromLatin1("list-level-properties"));
    writer.writeAttribute(foNS, QString::fromLatin1("text-align"), QString::fromLatin1("start"));
    QString spacing = QString::fromLatin1("%1mm").arg(format.indent() * 8);
    writer.writeAttribute(textNS, QString::fromLatin1("space-before"), spacing);
    //writer.writeAttribute(textNS, QString::fromLatin1("min-label-width"), spacing);

    writer.writeEndElement(); // list-level-style-*
    writer.writeEndElement(); // list-style
}
void ODTWriter::writeFrameFormat(QXmlStreamWriter &writer, QTextFrameFormat format, int formatIndex) const
{
    writer.writeStartElement(styleNS, QString::fromLatin1("style"));
    writer.writeAttribute(styleNS, QString::fromLatin1("name"), QString::fromLatin1("s%1").arg(formatIndex));
    writer.writeAttribute(styleNS, QString::fromLatin1("family"), QString::fromLatin1("section"));
    writer.writeEmptyElement(styleNS, QString::fromLatin1("section-properties"));
    if (format.hasProperty(QTextFormat::BlockTopMargin))
        writer.writeAttribute(foNS, QString::fromLatin1("margin-top"), pixelToPoint(qMax(qreal(0.), format.topMargin())) );
    if (format.hasProperty(QTextFormat::BlockBottomMargin))
        writer.writeAttribute(foNS, QString::fromLatin1("margin-bottom"), pixelToPoint(qMax(qreal(0.), format.bottomMargin())) );
    if (format.hasProperty(QTextFormat::BlockLeftMargin))
        writer.writeAttribute(foNS, QString::fromLatin1("margin-left"), pixelToPoint(qMax(qreal(0.), format.leftMargin())) );
    if (format.hasProperty(QTextFormat::BlockRightMargin))
        writer.writeAttribute(foNS, QString::fromLatin1("margin-right"), pixelToPoint(qMax(qreal(0.), format.rightMargin())) );

    writer.writeEndElement();
}
void ODTWriter::writeTableCellFormat(QXmlStreamWriter &writer, QTextTableCellFormat format, int formatIndex) const
{
    writer.writeStartElement(styleNS, QString::fromLatin1("style"));
    writer.writeAttribute(styleNS, QString::fromLatin1("name"), QString::fromLatin1("T%1").arg(formatIndex));
    writer.writeAttribute(styleNS, QString::fromLatin1("family"), QString::fromLatin1("table"));
    writer.writeEmptyElement(styleNS, QString::fromLatin1("table-properties"));

    qreal padding = format.topPadding();
    if(padding >0 && padding == format.bottomPadding() && padding == format.leftPadding() && padding == format.rightPadding())
    {
        writer.writeAttribute(foNS, QString::fromLatin1("padding"), pixelToPoint(padding));
    }
    else
    {
        if (padding > 0)
            writer.writeAttribute(foNS, QString::fromLatin1("padding-top"), pixelToPoint(padding));
        if (format.bottomPadding() > 0)
            writer.writeAttribute(foNS, QString::fromLatin1("padding-bottom"), pixelToPoint(format.bottomPadding()));
        if (format.leftPadding() > 0)
            writer.writeAttribute(foNS, QString::fromLatin1("padding-left"), pixelToPoint(format.leftPadding()));
        if (format.rightPadding() > 0)
            writer.writeAttribute(foNS, QString::fromLatin1("padding-right"), pixelToPoint(format.rightPadding()));
    }

    if (format.hasProperty(QTextFormat::TextVerticalAlignment))
    {
        QString pos;
        switch(format.verticalAlignment())
        {
        case QTextCharFormat::AlignMiddle:
            pos = QString::fromLatin1("middle"); break;
        case QTextCharFormat::AlignTop:
            pos = QString::fromLatin1("top"); break;
        case QTextCharFormat::AlignBottom:
            pos = QString::fromLatin1("bottom"); break;
        default:
            pos = QString::fromLatin1("automatic"); break;
        }
        writer.writeAttribute(styleNS, QString::fromLatin1("vertical-align"), pos);
    }
    writer.writeEndElement();
}
void ODTWriter::writeFrame(QXmlStreamWriter &writer, const QTextFrame *frame)
{
    const QTextTable *table = qobject_cast<const QTextTable *>(frame);

    if (table)
    {
        writer.writeStartElement(tableNS, QString::fromLatin1("table"));
        writer.writeEmptyElement(tableNS, QString::fromLatin1("table-column"));
        writer.writeAttribute(tableNS, QString::fromLatin1("number-columns-repeated"), QString::number(table->columns()));
    } else if (frame->document() && frame->document()->rootFrame() != frame){
        writer.writeStartElement(textNS, QString::fromLatin1("section"));
    }

    QTextFrame::iterator iterator = frame->begin();
    QTextFrame *child = 0;

    int tableRow = -1;
    while(!iterator.atEnd())
    {
        if (iterator.currentFrame() && child!=iterator.currentFrame())
            writeFrame(writer, iterator.currentFrame());
        else
        {
            QTextBlock block = iterator.currentBlock();
            if(table)
            {
                QTextTableCell cell = table->cellAt(block.position());
                if(tableRow < cell.row())
                {
                    if (tableRow >= 0)
                        writer.writeEndElement();
                    tableRow = cell.row();
                    writer.writeStartElement(tableNS, QString::fromLatin1("table-row"));
                }

                writer.writeStartElement(tableNS, QString::fromLatin1("table-cell"));
                if(cell.columnSpan() > 1)
                    writer.writeAttribute(tableNS, QString::fromLatin1("number-columns-spanned"), QString::number(cell.columnSpan()));
                if(cell.rowSpan()>1)
                    writer.writeAttribute(tableNS, QString::fromLatin1("number-rows-spanned"), QString::number(cell.rowSpan()));
                if(cell.format().isTableCellFormat())
                    writer.writeAttribute(tableNS, QString::fromLatin1("style-name"), QString::fromLatin1("T%1").arg(cell.tableCellFormatIndex()));
            }
            writeBlock(writer, block);
            if(table)
                writer.writeEndElement();
        }
        child = iterator.currentFrame();
        ++iterator;
    }
    if (tableRow >= 0)
        writer.writeEndElement();
    if (table || (frame->document() && frame->document()->rootFrame() != frame))
        writer.writeEndElement();
}

void ODTWriter::writeInlineCharacter(QXmlStreamWriter &writer, const QTextFragment &fragment)
{
    writer.writeStartElement(drawNS, QString::fromLatin1("frame"));
    if (fragment.charFormat().isImageFormat())
    {
        QTextImageFormat imageFormat = fragment.charFormat().toImageFormat();
        writer.writeAttribute(drawNS, QString::fromLatin1("name"), imageFormat.name());

        QImage image;
        QString name = imageFormat.name();
        if (name.startsWith(QLatin1String(":/")))
            name.prepend(QLatin1String("qrc"));
        QUrl url = QUrl::fromEncoded(name.toUtf8());
        const QVariant data = document.resource(QTextDocument::ImageResource, url);
        if (data.type() == QVariant::Image)
        {
            image = qvariant_cast<QImage>(data);
        } else if (data.type() == QVariant::ByteArray)
        {
            image.loadFromData(data.toByteArray());
        }

        if (image.isNull())
        {
            name = imageFormat.name();
            image.load(name);
        }

        if (!image.isNull())
        {
            QBuffer imageBytes;
            QImageWriter imageWriter(&imageBytes, "png");
            imageWriter.write(image);
            const QString fileName = createUniquePictureName();
            addFile(fileName, QString::fromLatin1("image/png"), imageBytes.data());

            qreal width = (imageFormat.hasProperty(QTextFormat::ImageWidth)) ? imageFormat.width() : image.width();
            writer.writeAttribute(svgNS, QString::fromLatin1("width"), pixelToPoint(width));
            qreal height = (imageFormat.hasProperty(QTextFormat::ImageHeight)) ? imageFormat.height() : image.height();
            writer.writeAttribute(svgNS, QString::fromLatin1("height"), pixelToPoint(height));

            writer.writeStartElement(drawNS, QString::fromLatin1("image"));
            writer.writeAttribute(xlinkNS, QString::fromLatin1("href"), fileName);
            writer.writeEndElement();
        }
    }

    writer.writeEndElement();
}

void ODTWriter::writeBufferToFile(QString &content, ZipWriter::CompressionPolicy cp, const QString &mimeType, const QString &fileName)
{
    zip->setCompressionPolicy(cp);
    addFile(fileName, mimeType, content.toUtf8());
}

void ODTWriter::writeMimeType()
{
    zip->setCompressionPolicy(ZipWriter::NeverCompress);
    zip->addFile(QString::fromLatin1("mimetype"), QByteArray("application/vnd.oasis.opendocument.text"));
}

void ODTWriter::writeContent()
{
    writeBufferToFile(content, ZipWriter::AutoCompress, QString::fromLatin1("text/xml"), QString::fromLatin1("content.xml"));
}

void ODTWriter::writeStyles()
{
    writeBufferToFile(styles, ZipWriter::AutoCompress, QString::fromLatin1("text/xml"), QString::fromLatin1("styles.xml"));
}

void ODTWriter::writeMeta()
{
    writeBufferToFile(meta, ZipWriter::AutoCompress, QString::fromLatin1("text/xml"), QString::fromLatin1("meta.xml"));
}

void ODTWriter::writeSettings()
{
    writeBufferToFile(settings, ZipWriter::AutoCompress, QString::fromLatin1("text/xml"), QString::fromLatin1("settings.xml"));
}

void ODTWriter::writeManifest()
{
    writeBufferToFile(manifest, ZipWriter::AutoCompress, QString::fromLatin1("text/xml"), QString::fromLatin1("META-INF/manifest.xml"));
}

QString ODTWriter::createUniquePictureName()
{
    return QString::fromLatin1("Pictures/Picture%1").arg(picCounter++);
}

void ODTWriter::addFile(const QString &fileName, const QString &mimeType, const QByteArray &bytes)
{
    zip->addFile(fileName, bytes);
    addFile(fileName, mimeType);
}

void ODTWriter::addFile(const QString &fileName, const QString &mimeType)
{
    manifestWriter.writeEmptyElement(manifestNS, QString::fromLatin1("file-entry"));
    manifestWriter.writeAttribute(manifestNS, QString::fromLatin1("media-type"), mimeType);
    manifestWriter.writeAttribute(manifestNS, QString::fromLatin1("full-path"), fileName);
}

void ODTWriter::writeNameSpaces(QXmlStreamWriter &writer)
{
    writer.writeNamespace(officeNS, QString::fromLatin1("office"));
    writer.writeNamespace(metaNS, QString::fromLatin1("meta"));
    writer.writeNamespace(configNS, QString::fromLatin1("config"));
    writer.writeNamespace(textNS, QString::fromLatin1("text"));
    writer.writeNamespace(tableNS, QString::fromLatin1("table"));
    writer.writeNamespace(drawNS, QString::fromLatin1("draw"));
    writer.writeNamespace(presentationNS, QString::fromLatin1("presentation"));
    writer.writeNamespace(chartNS, QString::fromLatin1("chart"));
    writer.writeNamespace(formNS, QString::fromLatin1("form"));
    writer.writeNamespace(scriptNS, QString::fromLatin1("script"));
    writer.writeNamespace(styleNS, QString::fromLatin1("style"));
    writer.writeNamespace(numberNS, QString::fromLatin1("number"));
    writer.writeNamespace(animNS, QString::fromLatin1("anim"));
    writer.writeNamespace(dcNS, QString::fromLatin1("dc"));
    writer.writeNamespace(xlinkNS, QString::fromLatin1("xlink"));
    writer.writeNamespace(mathNS, QString::fromLatin1("math"));
    writer.writeNamespace(xformNS, QString::fromLatin1("xform"));
    writer.writeNamespace(foNS, QString::fromLatin1("fo"));
    writer.writeNamespace(svgNS, QString::fromLatin1("svg"));
    writer.writeNamespace(smilNS, QString::fromLatin1("smil"));
}
