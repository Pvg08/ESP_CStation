#ifndef SERVER_H
#define SERVER_H

#include <QObject>
#include <QTcpServer>
#include <QNetworkSession>
#include <QtNetwork>
#include <stdlib.h>

class Server : public QObject
{
    Q_OBJECT

public:
    Server();
    ~Server();
    void StartServer(int server_port);

signals:
    void error(QString message);
    void write_message(QString message);

private slots:
    void sessionOpened();
    void sendData();
    void recieveData();
    void displayError(QAbstractSocket::SocketError socketError);
private:
    int port = 0;
    QMap<QString, QTcpSocket *> *sockets;
    QString ipAddress;
    QTcpServer *tcpServer;
    QNetworkSession *networkSession;
};

#endif
