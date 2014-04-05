#include <QLocale>

#include "spellcheckselectordialog.h"
#include "ui_spellcheckselectordialog.h"

#include "configuration.h"

SpellCheckSelectorDialog::SpellCheckSelectorDialog(QWidget *parent, Qt::WindowFlags f) :
    QDialog(parent, f),
    ui(new Ui::SpellCheckSelectorDialog)
{
    ui->setupUi(this);
    Configuration *conf = Configuration::getInstance();
    const QStringList sl = conf->getAllAvailableSpellCheckDictNames();
    int index = -1;
    QLocale locale;
    for(int i=0; i<sl.length(); i++){
        QString dictName = sl.at(i);
        if(dictName=="en_GB"){
            ui->languageComboBox->addItem("English - United Kingdom", dictName);
        } else if(dictName=="en_US"){
            ui->languageComboBox->addItem("English - United States", dictName);
        } else {
            ui->languageComboBox->addItem(dictName, dictName);
        }
        if(dictName==locale.name())
            index = i;
    }
    if(index==-1 && sl.length()>=1){
        ui->languageComboBox->setCurrentIndex(0);
    }

    connect(ui->buttonBox, SIGNAL(rejected()),
            this, SLOT(close()));
    connect(ui->buttonBox, SIGNAL(accepted()),
            this, SLOT(acceptSlot()));
}

void SpellCheckSelectorDialog::acceptSlot()
{
    language = ui->languageComboBox->itemData(ui->languageComboBox->currentIndex()).toString();
    accept();;
}

QString SpellCheckSelectorDialog::getSpellCheckLanguageName()
{
    return language;
}

SpellCheckSelectorDialog::~SpellCheckSelectorDialog()
{
    delete ui;
}
