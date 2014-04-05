#ifndef NOTICEDIALOG_H
#define NOTICEDIALOG_H

#include <QDialog>

namespace Ui {
class NoticeDialog;
}

class NoticeDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit NoticeDialog(QWidget *parent = 0);
    void setNoticeContent(const QString &content);
    ~NoticeDialog();
    
private:
    Ui::NoticeDialog *ui;
};

#endif // NOTICEDIALOG_H
