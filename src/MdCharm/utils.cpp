#include "utils.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QObject>
#include <QtCore>
#include <QProcess>
#include <QLatin1String>
#include <QDesktopServices>
#include <QSettings>
#include <QFileInfoList>

#include "configuration.h"
#include "resource.h"
#include "util/spellcheck/spellchecker.h"

//------------------------ MdCharmGlobal ---------------------------------------

MdCharmGlobal *MdCharmGlobal::instance = NULL;

MdCharmGlobal* MdCharmGlobal::getInstance()
{
    if(instance==NULL)
        instance = new MdCharmGlobal();
    return instance;
}

MdCharmGlobal::MdCharmGlobal()
{
    conf = Configuration::getInstance();
    MdCharm_ShortCut_Close_Tab = QKeySequence(conf->getKeyboardShortcut(ShortcutCloseTab));
    MdCharm_ShortCut_Italic = QKeySequence(conf->getKeyboardShortcut(ShortcutItalic));
    MdCharm_ShortCut_Bold = QKeySequence(conf->getKeyboardShortcut(ShortcutBold));
    MdCharm_ShortCut_Quote_Text = QKeySequence(conf->getKeyboardShortcut(ShortcutQuoteText));
    MdCharm_ShortCut_Shift_Tab = QKeySequence(conf->getKeyboardShortcut(ShortcutShiftTab));
    MdCharm_ShortCut_Strike_Through = QKeySequence(conf->getKeyboardShortcut(ShortcutStrikeThrough));
    MdCharm_ShortCut_Insert_Link = QKeySequence(conf->getKeyboardShortcut(ShortcutInsertLink));
    MdCharm_ShortCut_Insert_Picture = QKeySequence(conf->getKeyboardShortcut(ShortcutInsertPicture));
    MdCharm_ShortCut_Cheat_Sheet = QKeySequence(conf->getKeyboardShortcut(ShortcutCheatSheet));
    MdCharm_ShortCut_Insert_Code = QKeySequence(conf->getKeyboardShortcut(ShortcutInsertCode));
    MdCharm_ShortCut_New = QKeySequence(conf->getKeyboardShortcut(ShortcutNew));
    MdCharm_ShortCut_Open = QKeySequence(conf->getKeyboardShortcut(ShortcutOpen));
    MdCharm_ShortCut_Save = QKeySequence(conf->getKeyboardShortcut(ShortcutSave));
    MdCharm_ShortCut_SaveAs = QKeySequence(conf->getKeyboardShortcut(ShortcutSaveAs));
    MdCharm_ShortCut_Print = QKeySequence(conf->getKeyboardShortcut(ShortcutPrint));
    MdCharm_ShortCut_Quit = QKeySequence(conf->getKeyboardShortcut(ShortcutQuit));
    MdCharm_ShortCut_Undo = QKeySequence(conf->getKeyboardShortcut(ShortcutUndo));
    MdCharm_ShortCut_Redo = QKeySequence(conf->getKeyboardShortcut(ShortcutRedo));
    MdCharm_ShortCut_Cut = QKeySequence(conf->getKeyboardShortcut(ShortcutCut));
    MdCharm_ShortCut_Copy = QKeySequence(conf->getKeyboardShortcut(ShortcutCopy));
    MdCharm_ShortCut_Paste = QKeySequence(conf->getKeyboardShortcut(ShortcutPaste));
    MdCharm_ShortCut_Select_All = QKeySequence(conf->getKeyboardShortcut(ShortcutSelectAll));
    MdCharm_ShortCut_Find = QKeySequence(conf->getKeyboardShortcut(ShortcutFind));
    MdCharm_ShortCut_Find_Previous = QKeySequence(conf->getKeyboardShortcut(ShortcutFindPrevious));
    MdCharm_ShortCut_Find_Next = QKeySequence(conf->getKeyboardShortcut(ShortcutFindNext));
    MdCharm_ShortCut_Help_Contents = QKeySequence(conf->getKeyboardShortcut(ShortcutHelpContents));
    MdCharm_ShortCut_Print_Preview = QKeySequence(conf->getKeyboardShortcut(ShortcutPrintPreview));
    MdCharm_ShortCut_Hide_Project_DockBar = QKeySequence(conf->getKeyboardShortcut(ShortcutHideProjectDockBar));

    //Spell Checker
    spellCheckerLanguageList = conf->getAllAvailableSpellCheckDictNames();
    foreach (QString dictName, spellCheckerLanguageList) {
        QLocale dictLocale(dictName);
        cacheSpellCheckDictLocaleName.insert(dictName,
                                             QString("%1 (%2-%3)")
                                             .arg(dictName)
                                             .arg(dictLocale.nativeLanguageName())
                                             .arg(dictLocale.nativeCountryName()));
    }
    loadSpellCheck(conf->getSpellCheckLanguage());
}

MdCharmGlobal::~MdCharmGlobal()
{
    foreach (SpellChecker *sc, spellCheckerManager.values()) {
        delete sc;
    }
}

bool MdCharmGlobal::loadSpellCheck(const QString &lan)
{
    if(spellCheckerManager.contains(lan)){
       return true;
    }
    if(conf->isCheckSpell() &&
            !lan.isEmpty() &&
            !spellCheckerLanguageList.isEmpty()&&
            spellCheckerLanguageList.indexOf(lan)!=-1){
        QString filePath = conf->getLanguageSpellCheckDictPath(lan);
        if(!filePath.isEmpty()){
            QString withoutSuffixFilePath = filePath.left(filePath.lastIndexOf('.'));
            spellCheckerManager[lan] = new SpellChecker(withoutSuffixFilePath, conf->getLanguageSpellCheckUserDictPath(), lan);
            return true;
        } else {
            conf->setCheckSpell(false);
            conf->setSpellCheckLanguage(QString());
            return false;
        }
    } else {
        return false;
    }
}

SpellChecker* MdCharmGlobal::getSpellChecker(const QString &lan)
{
    if(lan.isEmpty())
        return NULL;
    if(spellCheckerManager.contains(lan))
        return spellCheckerManager.value(lan);
    if(spellCheckerLanguageList.indexOf(lan)==-1)
        return NULL;
    if(!loadSpellCheck(lan))
        return NULL;
    return spellCheckerManager.value(lan);
}

QStringList MdCharmGlobal::getSpellCheckerLanguageList()
{
    return spellCheckerLanguageList;
}

QString MdCharmGlobal::getDictLocaleName(const QString &dictName)
{
    return cacheSpellCheckDictLocaleName.value(dictName);
}

QString MdCharmGlobal::getShortDescriptionText(int s)
{
    switch (s) {
        case ShortcutCloseTab:
            return tr("Close tab");
            break;
        case ShortcutItalic:
            return tr("Italic text");
            break;
        case ShortcutBold:
            return tr("Bold text");
            break;
        case ShortcutQuoteText:
            return tr("Quote a block of text");
            break;
        case ShortcutShiftTab:
            return tr("Untab a block of text");
            break;
        case ShortcutStrikeThrough:
            return tr("Strike through text");
            break;
        case ShortcutInsertLink:
            return tr("Insert link");
            break;
        case ShortcutInsertPicture:
            return tr("Insert picture");
            break;
        case ShortcutCheatSheet:
            return tr("Open cheat-sheet");
            break;
        case ShortcutInsertCode:
            return tr("Insert code");
            break;
        case MdCharmGlobal::ShortcutNew:
            return tr("New");
            break;
        case MdCharmGlobal::ShortcutOpen:
            return tr("Open");
            break;
        case MdCharmGlobal::ShortcutSave:
            return tr("Save");
            break;
        case MdCharmGlobal::ShortcutSaveAs:
            return tr("SaveAs");
            break;
        case MdCharmGlobal::ShortcutPrint:
            return tr("Print");
            break;
        case MdCharmGlobal::ShortcutQuit:
            return tr("Quit");
            break;
        case MdCharmGlobal::ShortcutUndo:
            return tr("Undo");
            break;
        case MdCharmGlobal::ShortcutRedo:
            return tr("Redo");
            break;
        case MdCharmGlobal::ShortcutCut:
            return tr("Cut");
            break;
        case MdCharmGlobal::ShortcutCopy:
            return tr("Copy");
            break;
        case MdCharmGlobal::ShortcutPaste:
            return tr("Paste");
            break;
        case MdCharmGlobal::ShortcutSelectAll:
            return tr("Select All");
            break;
        case MdCharmGlobal::ShortcutFind:
            return tr("Find");
            break;
        case MdCharmGlobal::ShortcutFindPrevious:
            return tr("Find Previous");
            break;
        case MdCharmGlobal::ShortcutFindNext:
            return tr("Find Next");
            break;
        case MdCharmGlobal::ShortcutHelpContents:
            return tr("Help");
            break;
        case MdCharmGlobal::ShortcutPrintPreview:
            return tr("Print Preview");
            break;
        case MdCharmGlobal::ShortcutHideProjectDockBar:
            return tr("Hide Project Dockbar");
            break;
        default:
            Q_ASSERT(0 && "This should not be happend");
            return "";
            break;
    }
}

//------------------ Utils -----------------------------------------------------

QString Utils::AppName = QString::fromLatin1("MdCharm");
const char* Utils::AppNameCStr = "MdCharm";

QString Utils::getSaveFileName(const QString &suffix, QWidget *parent,
                               const QString &caption, const QString &dir,
                               const QString &filter, QString *selectedFilter)
{
    QString filePath;
#if defined(Q_OS_WIN)
    Q_UNUSED(suffix)
    filePath = QFileDialog::getSaveFileName(parent,
                                            caption,
                                            dir,
                                            filter,
                                            selectedFilter);
#elif defined(Q_OS_LINUX)
    //TODO: multi suffix
    do
    {
        filePath = QFileDialog::getSaveFileName(parent,
                                                caption,
                                                dir,
                                                filter,
                                                selectedFilter,
                                                QFileDialog::DontConfirmOverwrite);
        if(filePath.isEmpty())
            break; //cancel
        if(QFileInfo(filePath).suffix().isEmpty())
            filePath.append(suffix);
        QString fileName = QFileInfo(filePath).fileName();
        QFile confirmFile(filePath);
        if(confirmFile.exists())//confirm overwrite
        {
            QMessageBox::StandardButton ret = QMessageBox::question(parent,
                                  QObject::tr("Save"),
                                  QObject::tr("A file named \"%1\" already exists.\n Do you want to replace it?").arg(fileName),
                                  QMessageBox::Yes|QMessageBox::No);
            if(ret==QMessageBox::Yes)
                break;
            else
                filePath.clear();
        }
    } while(filePath.isEmpty());
#elif defined(Q_OS_MAC)
    assert(0);//TODO: MAC OS X
#endif
    return filePath;
}

QString Utils::getExistingDirectory(QWidget *parent, const QString &caption,
                                    const QString &dir,
                                    QFileDialog::Options options)
{
    return QFileDialog::getExistingDirectory(parent, caption, dir, options).replace('\\','/');
}

bool Utils::saveFile(const QString &fileFullPath, const QString &content)
{
    if(fileFullPath.isEmpty())
        return false;
    QFile file(fileFullPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        Utils::showFileError(file.error(), fileFullPath);
        return false;
    }
    if(-1 == file.write(content.toLocal8Bit()) ) // is it ok ?
    {
        Utils::showFileError(file.error(), fileFullPath);
    }
    file.close();
    return true;
}

bool Utils::saveFile(const QString &fileFullPath, const QByteArray &content)
{
    if(fileFullPath.isEmpty())
        return false;
    QFile file(fileFullPath);
    if( !file.open(QIODevice::WriteOnly|QIODevice::Text))
    {
        Utils::showFileError(file.error(), fileFullPath);
        return false;
    }
    if(-1 == file.write(content) )
    {
        Utils::showFileError(file.error(), fileFullPath);
    }
    file.close();
    return true;
}

void Utils::showFileError(QFile::FileError error, const QString &filePath)
{
    QString path = filePath;
#ifdef Q_OS_WIN
    path.replace("/", "\\");//keep this
#endif
    QString tip;
    switch(error)
    {
    case QFile::NoError:
        return;
        break;
    case QFile::ReadError:
        tip = QObject::tr("Can't Read File %1").arg(path);
        break;
    case QFile::WriteError:
        tip = QObject::tr("Can't Write File %1").arg(path);
        break;
    case QFile::OpenError:
        tip = QObject::tr("Can't Open File %1").arg(path);
        break;
    case QFile::PermissionsError:
        tip = QObject::tr("Can't Open/Write This File %1. Permission Denied!").arg(path);
        break;
    default:
        tip = QObject::tr("Can't Open This File");
        break;
    }
    QMessageBox::critical(0, "File Error", tip);
}

QByteArray Utils::encodeString(const QString src, const QString &codecName, bool addBom)
{
    QTextEncoder *encoder;
    if(codecName.isEmpty())
        encoder = QTextCodec::codecForName("System")->makeEncoder();
    else
        encoder = QTextCodec::codecForName(codecName.toStdString().c_str())->makeEncoder();
    if(addBom)
        return encoder->fromUnicode(src);
    QByteArray source = encoder->fromUnicode(src);
    QByteArray bom;
    if(codecName == QString::fromLatin1("UTF-8"))
    {
        bom.resize(3);
        bom[0] = (unsigned char)0xEF;
        bom[1] = (unsigned char)0xBB;
        bom[2] = (unsigned char)0xBF;
    } else if (codecName == QString::fromLatin1("UTF-16LE"))
    {
        bom.resize(3);
        bom[0] = (unsigned char)0xFE;
        bom[1] = (unsigned char)0xFF;
        bom[2] = 0x00;
    } else if (codecName == QString::fromLatin1("UTF-16BE"))
    {
        bom.resize(3);
        bom[0] = (unsigned char)0xFF;
        bom[1] = (unsigned char)0xFE;
        bom[2] = 0x00;
    }
    //TODO: utf-32le and utf32-be
    if(source.startsWith(bom))
    {
        source = source.remove(0, bom.length());//remove bom
    }
    return source;
}

bool Utils::isUtf8WithoutBom(const QByteArray &content)
{
    if(content.isEmpty())
    {
        return false;
    }
    int start = 0;
    int end = content.length();
    bool isUTF8NoBom = true;
    while(start<end)
    {
        QChar c(content[start]);
        if(content[start] == QChar(0x0))
        {
            isUTF8NoBom = false;
            break;
        }
        else if(content[start]<QChar(0x80))// 1MMMNNNN
        {
            start++;
        }
        else if (content[start]<QChar(0x80+0x40))// 11MMNNNN 0xC0
        {
            isUTF8NoBom = false;
            break;
        }
        else if (content[start]<QChar(0x80+0x40+0x20))//111MNNNN 0xE0
        {
            if (start>=end-1)  
                break;
            if(content[start+1]<=QChar(0x80) || content[start+1]>=QChar(0xC0))
            {
                isUTF8NoBom = false;
                break;
            }
            start += 2;
        }
        else if (content[start] < QChar(0x80 + 0x40 + 0x20 + 0x10))//1111NNNN 0xF0
        {
            if (start>=end-2)
                break;
            //TODO: is last 2 char encoding in utf8?
            start += 3;
        }
        else
        {
            break;
        }
    }
    return isUTF8NoBom;
}

bool Utils::isMarkdownFile(const QString &fileName)
{
    QStringList exts = Configuration::getInstance()->getFileExtension();
    if(exts.length()<=Configuration::MarkdownFile)
        return false;
    QRegExp extRe(exts.value(Configuration::MarkdownFile).replace('.', "\\.").replace('*', ".*"), Qt::CaseInsensitive);
    extRe.setMinimal(false);
    return extRe.exactMatch(fileName);
}

QFileInfoList Utils::listAllFileInDir(const QString &dirPath, const QStringList &nameFilter, QDir::Filters filters, bool isListSubdir)
{
    QFileInfoList l;
    QDirIterator it(dirPath, nameFilter, filters, isListSubdir ? QDirIterator::Subdirectories : QDirIterator::NoIteratorFlags);
    while(it.hasNext()){
        it.next();
        l.append(it.fileInfo());
    }
    return l;
}

QString Utils::checkOrAppendDefaultSuffix(MdCharmGlobal::WikiType type, const QString &fileName)
{
    switch(type)
    {
    case MdCharmGlobal::Markdown:
        if(Utils::isMarkdownFile(fileName))
            return fileName;
        return fileName+".md";
    default:
        return fileName;
    }
}

QStringList Utils::getEncodingList()
{
    QList<QByteArray> cl = QTextCodec::availableCodecs();
    QStringList sl;
    for(int i=0; i<cl.length(); i++)
        sl.append(QString(cl.at(i)));
    sl.sort();
    return sl;
}

bool Utils::calculateBom(bool hasBom, const QString &encoding,MdCharmGlobal::UTF8BOM ub)
{
    if(encoding == QString::fromLatin1("UTF-8"))
        return hasBom;
    bool bom;
    switch(ub)
    {
        default:
        case MdCharmGlobal::Keep:
            bom = hasBom;
        case MdCharmGlobal::Add:
            bom = true;
        case MdCharmGlobal::Delete:
            bom = false;
    }
    return bom;
}

bool Utils::isLetterOrNumberString(const QString &str)
{
    if(str.isEmpty())
        return false;
//    std::string stdStr = str.toStdString();
//    const char* stdStrChars = stdStr.c_str();
//    for(int i=0; stdStrChars[i]!='\0'; i++){
//        char ascii = stdStrChars[i];
//        if(ascii<=0 || !isalnum(ascii))
//            return false;
//    }
    for(int i=0; i<str.length(); i++){
        QChar c = str.at(i);
//        char ascii = c.toAscii();
        if(!c.isLetterOrNumber())
            return false;
    }
    return true;
}

void Utils::showInGraphicalShell(const QString &path)
{
#if defined(Q_OS_WIN)
    QString param;
    if(!QFileInfo(path).isDir())
        param = QLatin1String("/select,");
    param += QDir::toNativeSeparators(path);
    QProcess::startDetached("explorer.exe", QStringList(param));
#elif defined(Q_OS_MAC)
    QStringList scriptArgs;
    scriptArgs << QLatin1String("-e")
               <<QString::fromLatin1("tell application \"Finder\" to reveal POSIX file \"%1\"")
                 .arg(path);
    QProcess::execute(QLatin1String("/usr/bin/osascript"), scriptArgs);
    scriptArgs.clear();
    scriptArgs << QLatin1String("-e")
               <<QLatin1String("tell application \"Finder\" to activate");
    QProcess::execute(QLatin1String("/usr/bin/osascript"), scriptArgs);
#else
    QFileInfo fileInfo(path);
    if(fileInfo.isDir())
        QDesktopServices::openUrl(fileInfo.absoluteFilePath());
    else
        QDesktopServices::openUrl(fileInfo.absolutePath());
#endif
}

QString Utils::translateMarkdown2Html(MarkdownToHtml::MarkdownType type, const QString &content)
{
    QByteArray src = content.toUtf8();
    std::string textResult;
    MarkdownToHtml::translateMarkdownToHtml(type, src.data(), src.length(), textResult);
    return QString::fromUtf8(textResult.c_str(), textResult.length());
}

QString Utils::readFile(const QString &filePath)
{
    QFile openFile(filePath);
    if (!openFile.open(QIODevice::ReadOnly))
    {
        Utils::showFileError(openFile.error(), filePath);
        return QString();
    }
    QTextStream openFileStream(&openFile);
    //autodetected
    QString fileContent = openFileStream.readAll();
    QByteArray cn = openFileStream.codec()->name();
    if(cn==QByteArray("System"))
    {//check if it is utf8 without bom
        openFile.seek(0);
        if(Utils::isUtf8WithoutBom(openFile.readAll()))
        {
            openFileStream.setCodec("UTF-8");
            openFileStream.seek(0);
            fileContent = openFileStream.readAll();
        }
    }
    openFile.close();
    return fileContent;
}

QString Utils::getHtmlTemplate()
{
    return Utils::readFile(":/markdown/markdown.html");
}
