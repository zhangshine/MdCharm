#include <QLocalSocket>
#include <QLocalServer>
#include <QStringList>
#include <QFile>
#include <QTextStream>

#include "mdcharmapplication.h"

MdCharmApplication::MdCharmApplication(int &argc, char **argv) :
    QApplication(argc, argv)
{
    _isRunning = false;
    QString serverName ="MdCharm-Server";
    QLocalSocket socket;
    socket.connectToServer(serverName);
    if(socket.waitForConnected(500)){
        QTextStream stream(&socket);
        QStringList args = arguments();
        if(args.count() > 1){
            args.removeFirst();//remove app self path
            stream << args.join(",");
        }
        else
            stream << QString();
        stream.flush();
        socket.waitForBytesWritten();
        _isRunning = true;
        return;
    }
    localServer = new QLocalServer(this);
    connect(localServer, SIGNAL(newConnection()),
            this, SLOT(newConnectionSlot()));
    if(!localServer->listen(serverName)){
        if(localServer->serverError()==QAbstractSocket::AddressInUseError
                && QFile::exists(localServer->serverName())){
            QFile::remove(localServer->serverName());
            localServer->listen(serverName);
        }
    }
}

void MdCharmApplication::newConnectionSlot()
{
    qDebug("new connection");
    QLocalSocket *socket = localServer->nextPendingConnection();
    if(!socket)
        return;
    socket->waitForReadyRead(1000);
    QTextStream stream(socket);
    QString args = stream.readAll();
    delete socket;
    emit openFiles(args.split(","));
}

bool MdCharmApplication::isRunning()
{
    return _isRunning;
}
