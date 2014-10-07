#ifndef INSERTLINKORPICTUREDIALOG_H
#define INSERTLINKORPICTUREDIALOG_H

#include <QDialog>

namespace Ui {
class InsertLinkOrPictureDialog;
}

class Configuration;

class InsertLinkOrPictureDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit InsertLinkOrPictureDialog(int type, const QString proDir=QString(), QWidget *parent = 0);
    ~InsertLinkOrPictureDialog();
    const QString getText() const;
    const QString getUrl() const;
    const QString getTitle() const;
    const QString getWidth() const;
    const QString getHeight() const;
    
private:
    Ui::InsertLinkOrPictureDialog *ui;
    int type;
    QString proDir;
    Configuration *conf;
private slots:
    void checkAndAccept();
    void showWarning(const QString &pw, const QString &lw);
    void insertLocalFileSlot();
};

#endif // INSERTLINKORPICTUREDIALOG_H
