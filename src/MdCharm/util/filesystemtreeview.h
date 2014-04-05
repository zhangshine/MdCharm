#ifndef FILESYSTEMTREEVIEW_H
#define FILESYSTEMTREEVIEW_H

#include <QTreeView>

class FileSystemTreeViewState
{
public:
    QList<QStringList>& getExpands();
    QStringList& getCurrentIndexStringList();
    void setCurrentIndexStringList(const QStringList &sl);
    int getVerticalBarState();
    void setVertialBarState(int v);
    int getHorizontalBarState();
    void setHorizontalBarState(int h);
private:
    QList<QStringList> expands;
    QStringList currentIndexStringList;
    int vbar;
    int hbar;
};

class FileSystemTreeView : public QTreeView
{
    Q_OBJECT
public:
    explicit FileSystemTreeView(QWidget *parent = 0);
    const QSet<QModelIndex>& getExpandsIndexs() const;
    QSet<QModelIndex>& getExpandsIndexs();
    void saveState(FileSystemTreeViewState &state);
    void loadState(FileSystemTreeViewState &state);
protected:
    virtual void reset();
signals:
    
public slots:
    void expandedTree(const QModelIndex &index);
    void collapsedTree(const QModelIndex &index);

private:
    QSet<QModelIndex> expandsIndexs;
    
};

#endif // FILESYSTEMTREEVIEW_H
