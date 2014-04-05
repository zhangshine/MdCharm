#ifndef ODTWRITER_H
#define ODTWRITER_H

#include <QSet>
#include <QStack>
#include <QtCore/QXmlStreamWriter>

#include "../zip/zipwriter.h"

class Configuration;
class QFile;
class QXmlStreamWriter;
class QTextBlock;
class QTextCharFormat;
class QTextListFormat;
class QTextFrameFormat;
class QTextTableCellFormat;
class QTextFrame;
class QTextFragment;
class QTextList;

class ODTWriter
{
public:
    ODTWriter(const QTextDocument &document, const QString &fileName);
    ~ODTWriter();

    bool writeAll();

    void writeBlock(QXmlStreamWriter &writer, const QTextBlock &block);
    void writeFormats(QXmlStreamWriter &writer, QSet<int> formats) const;
    void writeBlockFormat(QXmlStreamWriter &writer, QTextBlockFormat format, int formatIndex) const;
    void writeCharacterFormat(QXmlStreamWriter &writer, QTextCharFormat format, int formatIndex) const;
    void writeListFormat(QXmlStreamWriter &writer, QTextListFormat format, int formatIndex) const;
    void writeFrameFormat(QXmlStreamWriter &writer, QTextFrameFormat format, int formatIndex) const;
    void writeTableCellFormat(QXmlStreamWriter &writer, QTextTableCellFormat, int formatIndex) const;
    void writeFrame(QXmlStreamWriter &writer, const QTextFrame *frame);
    void writeInlineCharacter(QXmlStreamWriter &writer, const QTextFragment &fragment);

    void writeMimeType();
    void writeContent();
    void writeStyles();
    void writeMeta();
    void writeSettings();
    void writeManifest();
    void writeBufferToFile(QString &content, ZipWriter::CompressionPolicy cp, const QString &mimeType, const QString &fileName);

    QString createUniquePictureName();
    void addFile(const QString &fileName, const QString &mimeType, const QByteArray &bytes);
private:
    void addFile(const QString &fileName, const QString &mimeType);
    void writeNameSpaces(QXmlStreamWriter &writer);
private:
    QString content;
    QString manifest;
    QString meta;
    QString styles;
    QString settings;

    QXmlStreamWriter manifestWriter;
    QXmlStreamWriter contentWriter;
    QXmlStreamWriter metaWriter;
    QXmlStreamWriter stylesWriter;

    QString officeNS, metaNS, configNS, textNS, tableNS, drawNS, presentationNS,
            chartNS, formNS, scriptNS, styleNS, numberNS, animNS, dcNS, xlinkNS,
            mathNS, xformNS,foNS, svgNS, smilNS;
    QString manifestNS;

    ZipWriter *zip;
    const QTextDocument &document;

    QStack<QTextList *> m_listStack;

    Configuration *conf;

    int picCounter;
};

#endif // ODTWRITER_H
