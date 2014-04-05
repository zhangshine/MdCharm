#ifndef CONFIGUREDIALOG_H
#define CONFIGUREDIALOG_H

#include <QDialog>

class QListWidget;
class QStackedWidget;
class QDialogButtonBox;
class QAbstractButton;
class QListWidgetItem;
class StylesPage;
class EnvironmentPage;
class TextEditorPage;

class ConfigureDialog : public QDialog
{
    Q_OBJECT
public:
    enum ListIndex
    {
        ENVIRONMENT=0,
        TEXTEDITOR,
        STYLES
    };
public:
    ConfigureDialog(QWidget *parent = 0, Qt::WindowFlags f = Qt::WindowTitleHint|Qt::WindowSystemMenuHint);
private:
    void initPagesIndex();
    void initSignalsAndSlots();
    void savePageConfigurations();
    
signals:
    void updateConfiguration();
    void updateShortcut(int s, const QString &newShortcut);
public slots:
    void onButtonsClicked(QAbstractButton *button);
    void changePage(QListWidgetItem *current, QListWidgetItem *pre);
private:
    QListWidget *contentsWidget;
    QStackedWidget *pagesWidget;

    QDialogButtonBox *buttons;
    
    EnvironmentPage *envPage;
    TextEditorPage *textEditorPage;
    StylesPage *stylesPage;
};

#endif // CONFIGUREDIALOG_H
