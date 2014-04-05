#ifndef MDCHARMAPPLICATION_H
#define MDCHARMAPPLICATION_H

#include <QApplication>
#include <QLocalServer>

class MdCharmApplication : public QApplication
{
    Q_OBJECT
public:
    explicit MdCharmApplication(int& argc, char** argv);
    
signals:
    void openFiles(const QStringList &sl);
public:
    bool isRunning();
public slots:
private slots:
    void newConnectionSlot();
private:
    QLocalServer *localServer;
    bool _isRunning;
};

#endif // MDCHARMAPPLICATION_H
