#include "filesystemmodel.h"
#include "configuration.h"

#include <QFileInfo>

FileNode::FileNode(FileSystemModel *model):
    model(model),
    parent(0),
    children(0)
{

}

FileNode::FileNode(FileSystemModel *model, const QString &path, FileNode *parent) :
    model(model),
    parent(parent),
    children(0),
    path(path),
    fileInfo(path)
{
    if(fileInfo.isDir() && !path.isEmpty()){
        model->getFileWatcher()->addPath(path);
    }
}

FileNode::~FileNode()
{
    if(isDir() && !path.isEmpty()){
        model->getFileWatcher()->removePath(path);
    }
    if(children){
        qDeleteAll(children->begin(), children->end());
        delete children;
    }
}

QList<FileNode *>* FileNode::getChildren()
{
    if(children==NULL){
        children = new QList<FileNode *>();
        if(!path.isEmpty()){
            QFileInfo info(path);
            if(info.isDir()){
                QDir dir(path);
                foreach(QFileInfo childInfo, dir.entryInfoList(model->getFilter(), model->getSort())){
                    children->append(new FileNode(model, childInfo.absoluteFilePath(), this));
                }
            }
        }
    }
    return children;
}

FileNode* FileNode::getParent()
{
    return parent;
}

FileNode* FileNode::child(int row)
{
    return children->at(row);
}

int FileNode::row() const
{
    if(parent)
        parent->getChildren()->indexOf(const_cast<FileNode*>(this));
    return 0;
}

QString FileNode::getPath() const
{
    return path;
}

QString FileNode::getText() const
{
    if(parent && parent->getParent() == NULL){
        return fileInfo.absoluteFilePath();
    } else {
        return fileInfo.fileName();
    }
}

QString FileNode::getName() const
{
    if(parent && parent->getParent() == NULL){
        return fileInfo.absoluteFilePath();
    } else {
        Configuration *conf = Configuration::getInstance();
        if(conf->isHideFileExtensionInProjectDock()){
            QString name = fileInfo.baseName();
            if(name.isEmpty())
                return fileInfo.fileName();
            else
                return name;
        } else
            return fileInfo.fileName();
    }
}

bool FileNode::isDir() const
{
    return fileInfo.isDir();
}

bool FileNode::isFile() const
{
    return fileInfo.isFile();
}

QFileInfo FileNode::getFileInfo() const
{
    return fileInfo;
}

void FileNode::clear()
{
    if(children){
        qDeleteAll(children->begin(), children->end());
        children->clear();
    }
}

void FileNode::reload()
{
    clear();
    if(children==NULL)
        children = new QList<FileNode*>();
    if(!path.isEmpty()){
        fileInfo = QFileInfo(path);
        if(fileInfo.isDir()){
            QDir dir(path);
            foreach(QFileInfo childInfo, dir.entryInfoList(model->getFilter(), model->getSort())){
                children->append(new FileNode(model, childInfo.absoluteFilePath(), this));
            }
        }
    }
}

FileNode *FileNode::findPath(const QString &target)
{
    if(!target.startsWith(path))
        return NULL;
    if(path==target)
        return this;

    QStringList nameList = target.right(target.length()-path.length()).split("/", QString::SkipEmptyParts);
    FileNode *curParent = this;
    bool find = false;
    foreach(QString name, nameList){
        find = false;
        QList<FileNode*>* curChildren = curParent->getChildren();
        for(int i=0; i<curChildren->length(); i++){
            FileNode *node = curChildren->at(i);
            if(!node->isDir())
                continue;
            if(node->getText()==name){
                curParent = node;
                find=true;
                break;
            }
        }
        if(!find)
            return NULL;
    }
    return curParent;
}

FileSystemModel::FileSystemModel(QObject *parent) :
    QAbstractItemModel(parent),
    rootNode(new FileNode(this)),
    iconProvider(new QFileIconProvider),
    fileWatcher(new QFileSystemWatcher(this))
{
    filters = QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot;
    sorts = QDir::DirsFirst | QDir::Name |QDir::Type;
}

FileSystemModel::~FileSystemModel()
{
    delete rootNode;
    delete iconProvider;
    delete fileWatcher;
}

QFileSystemWatcher* FileSystemModel::getFileWatcher() const
{
    return fileWatcher;
}

void FileSystemModel::setFilter(QDir::Filters f)
{
    filters = f;
}

QDir::Filters FileSystemModel::getFilter() const
{
    return filters;
}

void FileSystemModel::setSortFlags(QDir::SortFlags flags)
{
    sorts = flags;
}

QDir::SortFlags FileSystemModel::getSort() const
{
    return sorts;
}

void FileSystemModel::setRootPath(const QString &path)
{
    startPath = path;
    beginResetModel();
    rootNode->clear();
    rootNode->getChildren()->append(new FileNode(this, path, rootNode));
    endResetModel();
}

QList<QModelIndex> FileSystemModel::findPaths(const QString &path) const
{
    QList<QModelIndex> list;
    QString curPath = QDir::fromNativeSeparators(QDir::cleanPath(path));
    for(int i=0; i<rowCount(); i++){
        QModelIndex find = findPathHelper(curPath, this->index(i, 0));
        if(find.isValid())
            list.append(find);
    }
    return list;
}

void FileSystemModel::directoryChanged(const QString &path)
{
    beginResetModel();
    QDir dir(path);
    bool b = dir.exists();
    if(!b)
        fileWatcher->removePath(path);
    foreach(QModelIndex index, findPaths(path)){
        FileNode *node = nodeFromIndex(index);
        if(b)
            node->reload();
        else if(node->getParent())
            node->getParent()->reload();
    }
    endResetModel();
}

FileNode* FileSystemModel::nodeFromIndex(const QModelIndex &index) const
{
    if(index.isValid())
        return static_cast<FileNode *>(index.internalPointer());
    return rootNode;
}

QFileInfo FileSystemModel::fileInfo(const QModelIndex &index) const
{
    return nodeFromIndex(index)->getFileInfo();
}

int FileSystemModel::rowCount(const QModelIndex &parent) const
{
    FileNode *node = nodeFromIndex(parent);
    return node->getChildren()->length();
}

int FileSystemModel::columnCount(const QModelIndex &) const
{
    return 1;
}

QModelIndex FileSystemModel::parent(const QModelIndex &child) const
{
    FileNode *node = nodeFromIndex(child);
    FileNode *parent = node->getParent();
    if(parent == rootNode)
        return QModelIndex();
    return createIndex(parent->row(), 0, parent);
}

QModelIndex FileSystemModel::index(int row, int column, const QModelIndex &parent) const
{
    if(!hasIndex(row, column, parent))
        return QModelIndex();
    FileNode *node = nodeFromIndex(parent);
    return createIndex(row, column, node->child(row));
}

QVariant FileSystemModel::data(const QModelIndex &index, int role) const
{
    FileNode *node = nodeFromIndex(index);
    if(!node)
        return QVariant();
    switch (role) {
        case Qt::DisplayRole:
#ifdef Q_OS_WIN
            return node->getName().replace("/", "\\");
#else
            return node->getName();
#endif
            break;
        case Qt::DecorationRole:
            return iconProvider->icon(node->getFileInfo());
            break;
        case Qt::FontRole:
            {
                QFont font;
                if(node->getPath()==startPath)
                    font.setBold(true);
                return font;
            }
            break;
        default:
            return QVariant();
            break;
    }
    return QVariant();
}

QModelIndex FileSystemModel::findPathHelper(const QString &path, const QModelIndex &parentIndex) const
{
    FileNode *node = nodeFromIndex(parentIndex);
    if(!path.startsWith(node->getPath()))
        return QModelIndex();
    if(path==node->getPath())
        return parentIndex;
    QStringList nameList = path.right(path.length()-node->getPath().length()).split("/", QString::SkipEmptyParts);
    QModelIndex parent = parentIndex;
    bool find = false;
    int count = nameList.count();
    for(int i=0; i<count; i++){
        find = false;
        for(int j=0; j<rowCount(parent); j++){
            QModelIndex index = this->index(j, 0, parent);
            FileNode *node = nodeFromIndex(index);
            if( ( ( i == count-1) || node->isDir()) && node->getText()==nameList.at(i)){
                parent = index;
                find = true;
                break;
            }
        }
        if(!find)
            return QModelIndex();
    }
    return parent;
}
