#ifndef ABSTRACTSERVER_H
#define ABSTRACTSERVER_H

#include <QObject>
#include <QHostAddress>

class AbstractServer : public QObject
{
    Q_OBJECT

public:
    AbstractServer(QObject* parent = 0);

    virtual void Reset() = 0;
    virtual void StartServer() = 0;
    virtual bool SendData(QString ip_to, QString message) = 0;
    virtual bool SendData(QHostAddress ip_to, QString message) = 0;
    virtual bool SendData(quint16 block_id, QString message) = 0;

signals:
    void error(QString message);
    void write_message(QString message);
};

#endif
