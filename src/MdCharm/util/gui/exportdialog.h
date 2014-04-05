#ifndef EXPORTDIALOG_H
#define EXPORTDIALOG_H

#include <QDialog>
#include <QStack>

namespace Ui {
class ExportDialog;
}

class Configuration;

class ExportDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit ExportDialog(QWidget *parent = 0);
    ~ExportDialog();
    void updateSelectAllCheckBoxStatus();
    void updateOtherLineEdits(const QString &filePath);

signals:
    void exportToHtml(const QString &filePath);
    void exportToPdf(const QString &filePath);
    void exportToOdt(const QString &filePath);
public slots:
    void exportOneFileFinished(bool isSuccess, bool addPer=true);//Export to one file finished
private slots:
    void htmlCheckBoxActionSlot(bool b);
    void pdfCheckBoxActionSlot(bool b);
    void odtCheckBoxActionSlot(bool b);
    void selectAllCheckBoxSlot(int state);
    void htmlPushButtonSlot();
    void pdfPushButtonSlot();
    void odtPushButtonSlot();
    void okButtonSlot();
    void startExport();
private:
    void restoreState();
    void saveState();
private:
    enum ExportType{
        PDF, ODT, HTML
    };
    
private:
    Ui::ExportDialog *ui;
    Configuration *conf;
    int per;
    QStack<ExportType> stack;
    QString log;
};

#endif // EXPORTDIALOG_H
