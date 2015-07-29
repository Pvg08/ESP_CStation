#ifndef SERVER_H
#define SERVER_H

#include <QObject>
#include <QTcpServer>
#include <QNetworkSession>
#include <QtNetwork>
#include <stdlib.h>
#include "clientblock.h"

class Server : public QObject
{
    Q_OBJECT

public:
    Server();
    ~Server();
    void Reset(int server_port);
    void StartServer(int server_port);
    bool SendData(QString ip_to, QString message);
    bool SendReboot(QString ip_to);
    bool SendRunSetup(QString ip_to);
    bool SendSetConfigsAndReset(QString ip_to, QString ssid, QString pssw, QString servip, quint8 stid);
    bool SendTone(QString ip_to, quint16 frequency, quint32 period);
    bool SendLCDText(QString ip_to, QString text);
    bool SendLCDReturn(QString ip_to);

    quint16 getNextBlockID(quint16 block_id);

    const QStringList getIPsList();
    ClientBlock *getClientBlock(quint32 ip_addr);
    ClientBlock *getClientBlockByID(quint16 block_id);

signals:
    void error(QString message);
    void write_message(QString message);
    void blocks_change();
    void sensors_change(quint16 block_id);
    void new_sensor(Sensor* sensor_obj);

private slots:
    void sessionOpened();
    void recieveConnection();
    void recieveData();
    void clientDisconnected();
    void socketStateChanged(QAbstractSocket::SocketState state);
    void displayError(QAbstractSocket::SocketError socketError);
    void clientBlockSensorsChange();
    void clientBlockNewSensor(Sensor* sensor_obj);
private:
    int port = 0;
    QMap<quint32, QTcpSocket *> *sockets;
    QMap<quint16, ClientBlock *> *clientblocks;
    QString ipAddress;
    QTcpServer *tcpServer;
    QNetworkSession *networkSession;
};

#endif
