#include <QtGui>

#include "configuredialog.h"
#include "pages.h"
#include "resource.h"

ConfigureDialog::ConfigureDialog(QWidget *parent, Qt::WindowFlags f) :
    QDialog(parent, f)
{
    contentsWidget = new QListWidget;
    contentsWidget->setViewMode(QListView::ListMode);
    contentsWidget->setIconSize(QSize(32, 32));
    contentsWidget->setMovement(QListView::Static);
    contentsWidget->setMaximumWidth(132);
    contentsWidget->setSpacing(2);

    pagesWidget = new QStackedWidget;
    envPage = new EnvironmentPage;
    pagesWidget->insertWidget(ENVIRONMENT, envPage);
    textEditorPage = new TextEditorPage;
    pagesWidget->insertWidget(TEXTEDITOR, textEditorPage);//ownership to QStackedWidget, no need to delete
    stylesPage = new StylesPage;
    pagesWidget->insertWidget(STYLES, stylesPage);

    buttons = new QDialogButtonBox(QDialogButtonBox::Ok|
                                   QDialogButtonBox::Cancel|
                                   QDialogButtonBox::Apply);

    QHBoxLayout *horizontalLayout = new QHBoxLayout;
    horizontalLayout->addWidget(contentsWidget);
    horizontalLayout->addWidget(pagesWidget, 1);

    QHBoxLayout *buttonsLayout = new QHBoxLayout;
    buttonsLayout->addWidget(buttons);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(horizontalLayout);
    mainLayout->addLayout(buttonsLayout);
    setLayout(mainLayout);

    setWindowTitle(tr("Preference"));
    initPagesIndex();
    initSignalsAndSlots();

    setMinimumSize(540, 400);
}

void ConfigureDialog::initPagesIndex()
{
    QListWidgetItem *envButton = new QListWidgetItem(contentsWidget);
    envButton->setText(tr("Environment"));
    envButton->setIcon(QIcon(Resource::EnvPageIcon));
    envButton->setTextAlignment(Qt::AlignLeft|Qt::AlignVCenter);
    envButton->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);

    QListWidgetItem *textEditorButton = new QListWidgetItem(contentsWidget);
    textEditorButton->setText(tr("Text Editor"));
    textEditorButton->setIcon(QIcon(Resource::TextEditorIcon));
    textEditorButton->setTextAlignment(Qt::AlignLeft|Qt::AlignVCenter);
    textEditorButton->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);

    QListWidgetItem *styleButton = new QListWidgetItem(contentsWidget);
    styleButton->setText(tr("Styles"));
    styleButton->setIcon(QIcon(Resource::StylesIcon));
    styleButton->setTextAlignment(Qt::AlignLeft|Qt::AlignVCenter);
    styleButton->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
}

void ConfigureDialog::initSignalsAndSlots()
{
    QObject::connect(buttons, SIGNAL(clicked(QAbstractButton*)),
                     this, SLOT(onButtonsClicked(QAbstractButton*)));
    QObject::connect(contentsWidget, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)),
                     this, SLOT(changePage(QListWidgetItem*,QListWidgetItem*)));
    QObject::connect(envPage, SIGNAL(updateShortcut(int,QString)),
                     this, SIGNAL(updateShortcut(int,QString)));
}

void ConfigureDialog::savePageConfigurations()
{
    envPage->saveConfig();
    textEditorPage->saveConfig();
    stylesPage->saveConfig();
}

void ConfigureDialog::onButtonsClicked(QAbstractButton *button)
{
    QDialogButtonBox::ButtonRole br = buttons->buttonRole(button);
    if(br == QDialogButtonBox::AcceptRole) // OK
    {
        savePageConfigurations();
        accept();
    } else if (br == QDialogButtonBox::RejectRole){// Cancel
        reject();
    } else if (br == QDialogButtonBox::ApplyRole){ // Apply
        savePageConfigurations();
        emit updateConfiguration();
    }
}

void ConfigureDialog::changePage(QListWidgetItem *current, QListWidgetItem *pre)
{
    if(!current)
        current = pre;
    pagesWidget->setCurrentIndex(contentsWidget->row(current));
}
