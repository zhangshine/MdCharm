#ifndef PROJECTDOCKWIDGET_H
#define PROJECTDOCKWIDGET_H

#include <QDockWidget>
#include <QtGui>
#include <QtCore>

#ifdef QT_V5
#include <QtWidgets>
#endif

class FileSystemModel;
class FileSystemTreeView;
class Configuration;

namespace Ui {
class ProjectDockWidget;
}

class ProjectDockWidget : public QDockWidget
{
    Q_OBJECT
    
public:
    explicit ProjectDockWidget(QWidget *parent = 0);
    void initGui();
    void initMenuAndAction();
    void initSignalsAndSlots();

    void setProjectDir(const QString &dirString);
    QString getProjectDir();

    ~ProjectDockWidget();
protected:
    virtual void keyPressEvent(QKeyEvent *event);
    virtual void dragEnterEvent(QDragEnterEvent *e);
    virtual void dropEvent(QDropEvent *e);
private:
    void prepareDirectoryMenu(const QString &dirPath);
    void prepareFileMenu(const QString &filePath);
signals:
    void openTheFile(const QString &filePath);
    void createNewFile(const QString &fileDir);
    void deleteFileSignal(const QString &filePath);
    void renameFileSignal(const QString &original, const QString &current);

private slots:
    void showContextMenu(const QPoint &point);
    void openFile();
    void showInExplorer();
    void createNewAux();
    void doubleClickedSlot(const QModelIndex &index);
    void deleteFile();
    void renameFile();
    void visibleChange(bool b);
    void directoryChanged(QString dir);
private:
    Ui::ProjectDockWidget *ui;
    FileSystemModel* fileSystemModel;
    FileSystemTreeView *projectTreeView;

    Configuration *conf;

    QMenu* fileMenu;
    QMenu* directoryMenu;
    QAction* addNewAction;
    QAction* showInExplorerAction;
    QAction* openFileAction;
    QAction* renameFileAction;
    QAction* deleteFileAction;

    QString projectDir;
};

#endif // PROJECTDOCKWIDGET_H
