#ifndef CHECKUPDATES_H
#define CHECKUPDATES_H

#include <QObject>
#include <QtNetwork>

class CheckUpdates : public QObject
{
    Q_OBJECT
public:
    explicit CheckUpdates(QObject *parent = 0);
    bool isHasError();

    
signals:
    void finished(const QString &info);
    
public slots:
    void error(QNetworkReply::NetworkError err);
    void httpFinished();
    void check();
private:
    QNetworkAccessManager manager;
    QNetworkReply *reply;
    bool hasError;
};

#endif // CHECKUPDATES_H
