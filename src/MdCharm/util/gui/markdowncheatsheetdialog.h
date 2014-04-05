#ifndef MARKDOWNCHEATSHEETDIALOG_H
#define MARKDOWNCHEATSHEETDIALOG_H

#include <QDialog>

#include "basewebview/markdownwebview.h"

class MarkdownCheatSheetDialog : public QDialog
{
    Q_OBJECT
public:
    explicit MarkdownCheatSheetDialog(QWidget *parent = 0);
    ~MarkdownCheatSheetDialog();
    void showAndPopup();
protected:
private:
    MarkdownWebView *webView;
};

#endif // MARKDOWNCHEATSHEETDIALOG_H
