#ifndef FILESYSTEMMODEL_H
#define FILESYSTEMMODEL_H

#include <QAbstractItemModel>
#include <QtCore>
#include <QtGui>

#ifdef QT_V5
#include <QtWidgets>
#endif

class FileSystemModel;
class FileNode
{
public:
    FileNode(FileSystemModel *model);
    FileNode(FileSystemModel *model, const QString &path, FileNode *parent);
    ~FileNode();
    FileNode* getParent();
    FileNode* child(int row);
    int row() const;
    QList<FileNode *>* getChildren();
    QString getPath() const;
    QString getText() const;
    QFileInfo fileInfo() const;
    bool isDir() const;
    bool isFile() const;
    void clear();
    void reload();
    FileNode* findPath(const QString &path);
private:
    FileSystemModel *model;
    FileNode *parent;
    QList<FileNode *> *children;
    QString path;
    QString text;
};

class FileSystemModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    explicit FileSystemModel(QObject *parent = 0);
    ~FileSystemModel();
    QFileSystemWatcher* getFileWatcher() const;
    void setFilter(QDir::Filters f);
    QDir::Filters getFilter() const;
    void setSortFlags(QDir::SortFlags flags);
    QDir::SortFlags getSort() const;
    void setRootPath(const QString &path);
    QList<QModelIndex> findPaths(const QString &path) const;
    FileNode* nodeFromIndex(const QModelIndex &index) const;
    QFileInfo fileInfo(const QModelIndex &index) const;
protected:
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
    virtual int columnCount(const QModelIndex &parent=QModelIndex()) const;
    virtual QModelIndex parent(const QModelIndex &child) const;
    virtual QModelIndex index(int row, int column, const QModelIndex &parent=QModelIndex()) const;
    virtual QVariant data(const QModelIndex &index, int role) const;
private:
    QModelIndex findPathHelper(const QString &path, const QModelIndex &parentIndex) const;
signals:
    
public slots:
    void directoryChanged(const QString& path);
private:
    FileNode *rootNode;
    QString startPath;
    QFileIconProvider *iconProvider;
    QFileSystemWatcher *fileWatcher;
    QDir::Filters filters;
    QDir::SortFlags sorts;
};

#endif // FILESYSTEMMODEL_H
