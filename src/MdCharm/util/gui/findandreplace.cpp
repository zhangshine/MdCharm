#include "findandreplace.h"

#include "ui_findandreplaceform.h"

FindAndReplace::FindAndReplace(QWidget *parent) :
    QWidget(parent), ui(new Ui::FindAndReplaceForm)
{
    ui->setupUi(this);
    closeFindButton = ui->closeFindToolButton;
    prevFindButton = ui->prevFindToolButton;
    nextFindButton = ui->nextFindToolButton;
    findTextLineEdit = ui->findTextLineEdit;

    connect(findTextLineEdit, SIGNAL(textChanged(QString)),
            this, SLOT(find(QString)));
    connect(closeFindButton, SIGNAL(clicked()),
            this, SLOT(hideFind()));
    connect(prevFindButton, SIGNAL(clicked()),
            this, SLOT(previous()));
    connect(nextFindButton, SIGNAL(clicked()),
            this, SLOT(next()));
    connect(findTextLineEdit, SIGNAL(returnPressed()),
            this, SLOT(next()));
    connect(ui->reCheckBox, SIGNAL(clicked(bool)),
            this, SLOT(regularExpressionCheckedSlot(bool)));
    connect(ui->excludePushButton, SIGNAL(clicked()),
            this, SLOT(next()));
    connect(ui->replacePushButton, SIGNAL(clicked()),
            this, SLOT(replaceButtonSlot()));
    connect(ui->replaceAllPushButton, SIGNAL(clicked()),
            this, SLOT(replaceAllButtonSlot()));
}

FindAndReplace::~FindAndReplace()
{
    delete ui;
    ui = NULL;
}

void FindAndReplace::regularExpressionCheckedSlot(bool b)
{
    if(b)
    {
        ui->wholeWordCheckBox->setChecked(false);
        ui->wholeWordCheckBox->setEnabled(false);
    } else {
        ui->wholeWordCheckBox->setEnabled(true);
    }
}

void FindAndReplace::find(const QString &ft, bool isSetCursor)
{
    QTextDocument::FindFlags ff;
    if(ui->caseSensitiveCheckBox->isChecked())
        ff |= QTextDocument::FindCaseSensitively;
    if(ui->wholeWordCheckBox->isChecked())
        ff |= QTextDocument::FindWholeWords;
    emit findText(ft, ff, ui->reCheckBox->isChecked(), isSetCursor);
}

void FindAndReplace::replaceButtonSlot()
{
    QTextDocument::FindFlags ff;
    if(ui->caseSensitiveCheckBox->isChecked())
        ff |= QTextDocument::FindCaseSensitively;
    if(ui->wholeWordCheckBox->isChecked())
        ff |= QTextDocument::FindWholeWords;
    emit replace(ui->replaceLineEdit->text());
}

void FindAndReplace::replaceAllButtonSlot()
{
    QTextDocument::FindFlags ff;
    if(ui->caseSensitiveCheckBox->isChecked())
        ff |= QTextDocument::FindCaseSensitively;
    if(ui->wholeWordCheckBox->isChecked())
        ff |= QTextDocument::FindWholeWords;
    emit replaceAll(ui->findTextLineEdit->text(), ui->replaceLineEdit->text(),
                    ff, ui->reCheckBox->isChecked());
}

void FindAndReplace::resendFindTextSignal()
{
    if(isVisible())
        find(ui->findTextLineEdit->text(), false);
}

void FindAndReplace::resendFindNextSignal()
{
    next();
}

void FindAndReplace::resendFindPreviousSignal()
{
    previous();
}

void FindAndReplace::setFindText(const QString &txt)
{
    if(txt.isEmpty())
        return;
    if(!findTextLineEdit->text().trimmed().isEmpty())
        return;
    findTextLineEdit->setText(txt);
}

void FindAndReplace::hideFind()
{
    findTextLineEdit->clear();
    emit findHide();
}

void FindAndReplace::previous()
{
    QTextDocument::FindFlags ff=QTextDocument::FindBackward;
    if(ui->reCheckBox->isChecked())
    {
        if(ui->caseSensitiveCheckBox->isChecked())
            ff = ff | QTextDocument::FindCaseSensitively;
        emit findPrevious(findTextLineEdit->text(),
                          ff, true);
        return;
    }
    //else
    if(ui->caseSensitiveCheckBox->isChecked())
    {
        ff = ff | QTextDocument::FindCaseSensitively;
    }
    if(ui->wholeWordCheckBox->isChecked())
    {
        ff = ff | QTextDocument::FindWholeWords;
    }
    emit findPrevious(findTextLineEdit->text(), ff, false);
}

void FindAndReplace::next()
{
    QTextDocument::FindFlags ff;
    if(ui->reCheckBox->isChecked())
    {
        if(ui->caseSensitiveCheckBox->isChecked())
            ff = ff | QTextDocument::FindCaseSensitively;
        emit findNext(findTextLineEdit->text(),
                      ff, true);
        return;
    }
    //else
    if(ui->caseSensitiveCheckBox->isChecked())
        ff = ff | QTextDocument::FindCaseSensitively;
    if(ui->wholeWordCheckBox->isChecked())
        ff = ff | QTextDocument::FindWholeWords;
    emit findNext(findTextLineEdit->text(), ff, false);
}

void FindAndReplace::setFocusTextEdit()
{
    findTextLineEdit->setFocus();
}
