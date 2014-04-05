#include "filesystemtreeview.h"

#include <QtGui>
#include <QtCore>

#ifdef QT_V5
#include <QtWidgets>
#endif

/***************************** FileSystemTreeViewState ******************/
QList<QStringList>& FileSystemTreeViewState::getExpands()
{
    return expands;
}

QStringList& FileSystemTreeViewState::getCurrentIndexStringList()
{
    return currentIndexStringList;
}

void FileSystemTreeViewState::setCurrentIndexStringList(const QStringList &sl)
{
    currentIndexStringList = sl;
}

int FileSystemTreeViewState::getVerticalBarState()
{
    return vbar;
}

void FileSystemTreeViewState::setVertialBarState(int v)
{
    vbar = v;
}

int FileSystemTreeViewState::getHorizontalBarState()
{
    return hbar;
}

void FileSystemTreeViewState::setHorizontalBarState(int h)
{
    hbar = h;
}

/***************************** Helper Function **************************/
static QStringList stringListFromIndex(const QModelIndex &index)
{
    QStringList list;
    if (!index.isValid())
        return list;
    list.append(stringListFromIndex(index.parent()));
    list.append(index.data().toString());
    return list;
}

static QModelIndex indexFromStringList(QAbstractItemModel *model, QStringList &list, const QModelIndex & parent = QModelIndex())
{
    if (list.isEmpty())
        return QModelIndex();
    QString text = list.front();
    for (int i = 0; i < model->rowCount(parent); i++) {
        QModelIndex child = model->index(i,0,parent);
        if (child.data().toString() == text) {
            list.pop_front();
            if (list.isEmpty()) {
                return child;
            } else {
                QModelIndex next = indexFromStringList(model,list,child);
                if (next.isValid())
                    return next;
                else
                    return child;
            }
        }
    }
    return QModelIndex();
}

/*************** FileSystemTreeView **************************************/
FileSystemTreeView::FileSystemTreeView(QWidget *parent) :
    QTreeView(parent)
{
    connect(this, SIGNAL(expanded(QModelIndex)),
            this, SLOT(expandedTree(QModelIndex)));
    connect(this, SIGNAL(collapsed(QModelIndex)),
            this, SLOT(collapsedTree(QModelIndex)));
}

void FileSystemTreeView::reset()
{
    QTreeView::reset();
    expandsIndexs.clear();
}

void FileSystemTreeView::saveState(FileSystemTreeViewState &state)
{
    state.getExpands().clear();
    QSetIterator<QModelIndex> i(getExpandsIndexs());
    while(i.hasNext())
        state.getExpands().append(stringListFromIndex(i.next()));
    state.setCurrentIndexStringList(stringListFromIndex(currentIndex()));
    state.setVertialBarState(verticalScrollBar()->value());
    state.setHorizontalBarState(horizontalScrollBar()->value());
}

void FileSystemTreeView::loadState(FileSystemTreeViewState &state)
{
    expandToDepth(0);
    foreach (QStringList sl, state.getExpands()) {
        QModelIndex expandIndex = indexFromStringList(model(), sl);
        if(expandIndex.isValid())
            setExpanded(expandIndex, true);
    }
    QModelIndex curIndex = indexFromStringList(model(), state.getCurrentIndexStringList());
    if(curIndex.isValid())
        setCurrentIndex(curIndex);
    verticalScrollBar()->setValue(state.getVerticalBarState());
    horizontalScrollBar()->setValue(state.getHorizontalBarState());
}

const QSet<QModelIndex>& FileSystemTreeView::getExpandsIndexs() const
{
    return expandsIndexs;
}

QSet<QModelIndex>& FileSystemTreeView::getExpandsIndexs()
{
    return expandsIndexs;
}

void FileSystemTreeView::expandedTree(const QModelIndex &index)
{
    expandsIndexs.insert(index);
}

void FileSystemTreeView::collapsedTree(const QModelIndex &index)
{
    expandsIndexs.remove(index);
}
