#include <cassert>
#include <QFileInfo>

#include "editareawidget.h"
#include "configuration.h"

/************************* FileModel ******************************************/
FileModel::FileModel()
{
    hasBom = false;
}

QString FileModel::getFileName()
{
    if (fileFullPath.isEmpty())
        return QString();
    return QFileInfo(fileFullPath).fileName();
}

void FileModel::setFileFullPath(const QString filePath){
    fileFullPath = filePath;
}
void FileModel::setHasBom(bool b) { hasBom = b; }
void FileModel::setEncodingFormatName(const QString c){ encodingFormatName = c; }
QString FileModel::getEncodingFormatName() { return encodingFormatName; }

bool FileModel::isHasBom() { return hasBom; }
QString FileModel::getFileFullPath(){ return fileFullPath; }
bool FileModel::isUntitled(){ return fileFullPath.isEmpty(); }

/************************* EditorModel ****************************************/
void EditorModel::setFindVisible(bool f){ findVisible=f; }
bool EditorModel::isFindVisible(){ return findVisible; }
void EditorModel::setCanCopy(bool c){ canCopy = c; }
bool EditorModel::isCopyAvailable(){ return canCopy; }
void EditorModel::setCurrentLineNumber(int l) { currentLineNumber = l; }
int EditorModel::getCurrentLineNumber() { return currentLineNumber; }
void EditorModel::setCurrentColumnNumber(int c) { currentColumnNumber = c; }
int EditorModel::getCurrentColumnNumber() { return currentColumnNumber; }
void EditorModel::overWriteChanged() { overWrite = !overWrite; }
bool EditorModel::isOverWrite() { return overWrite; }
void EditorModel::setEditorType(EditorType t) { editorType = t; }
EditorModel::EditorType EditorModel::getEditorType() { return editorType; }
bool EditorModel::isEditable()
{
    if(editorType>EDITABLE)
        return true;
    return false;
}

/************************* StateModel *****************************************/
bool StateModel::isValid(){return !fileFullPath.isEmpty();}
bool StateModel::isCurrentTab(){return currentTab; }
void StateModel::setIsCurrentTab(bool b){currentTab=b;}
int StateModel::getSelectionStart()const{return selectionStart;}
void StateModel::setSelectionStart(int start){selectionStart = start;}
int StateModel::getSelectionEnd()const{return selectionEnd;}
void StateModel::setSelectionEnd(int end){selectionEnd = end;}
int StateModel::getFirstVisibleLine()const{return firstVisibleLine;}
void StateModel::setFirstVisibleLine(int line){firstVisibleLine = line;}
const QString StateModel::getFileFullPath()const{return fileFullPath;}
void StateModel::setFileFullPath(const QString &ffp){fileFullPath=ffp;}
int StateModel::getVerticalScorllBarCurrentValue()const {return verticalScrollBarCurrentValue;}
void StateModel::setVerticalScrllBarCurrentValue(int value){verticalScrollBarCurrentValue=value;}
int StateModel::getVerticalScrollBarMaxValue()const {return verticalScrollBarMaxValue;}
void StateModel::setVerticalScrollBarMaxValue(int value) {verticalScrollBarMaxValue = value;}
int StateModel::getViewNum(){ return viewNum; }
void StateModel::setViewNum(int v){ viewNum = v; }

EditAreaWidget::EditAreaWidget(const QString &filePath, EditActionOptions op) :
    QWidget(0),
    options(op)
{
    fm = QSharedPointer<FileModel>(new FileModel());
    fm->setFileFullPath(filePath);
    conf = Configuration::getInstance();
}

EditAreaWidget::EditAreaWidget(EditAreaWidget &e)
{
    fm = QSharedPointer<FileModel>(e.fm);
    conf = Configuration::getInstance();
    options = e.options;
    em = e.em;
}

void EditAreaWidget::setText(const QString &text)
{
    Q_UNUSED(text);
}

QString EditAreaWidget::getText()
{
    return QString();
}

void EditAreaWidget::exportToPdf(const QString &filePath)
{
    Q_UNUSED(filePath)
}

void EditAreaWidget::exportToODT(const QString &filePath)
{
    Q_UNUSED(filePath)
}

void EditAreaWidget::exportToHtml(const QString &filePath)
{
    Q_UNUSED(filePath)
}

void EditAreaWidget::printContent()
{
    assert("this method must not be called!");
}

void EditAreaWidget::printPreview()
{
    assert("this method must not be called!");
}

bool EditAreaWidget::saveFile()
{
    return false;//saved?
}

void EditAreaWidget::saveFileAs()
{

}

void EditAreaWidget::switchPreview(int isShowPreview)
{
    Q_UNUSED(isShowPreview);
}

void EditAreaWidget::changeSyncScrollbarSetting(bool sync)
{
    Q_UNUSED(sync);
}

void EditAreaWidget::reloadFile()
{
}

int EditAreaWidget::getCurrentMaxBlockCount()
{
    return 1;
}

void EditAreaWidget::gotoLine(int line)
{
    Q_UNUSED(line)
}

const StateModel EditAreaWidget::getState()
{
    return StateModel();
}

void EditAreaWidget::restoreFileState(const StateModel &sm)
{
    Q_UNUSED(sm)
}

void EditAreaWidget::changeFilePath(const QString &newFilePath)
{
    fm->setFileFullPath(newFilePath);
}

void EditAreaWidget::setCopyAvaliable(bool a)
{
    em.setCanCopy(a);
    emit updateActions();
}

FileModel EditAreaWidget::getFileModel()
{
    return *fm;
}

EditorModel EditAreaWidget::getEditorModel()
{
    return em;
}

void EditAreaWidget::setFileEncoding(const QString en)
{
    fm->setEncodingFormatName(en);
}

void EditAreaWidget::updateConfiguration()
{

}

void EditAreaWidget::findNext()
{}
void EditAreaWidget::findPrevious()
{}

bool EditAreaWidget::isEditActionOptionEnabled(EditActionOption op)
{
    return options.testFlag(op);
}
