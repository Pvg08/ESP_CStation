#include "server.h"

Server::Server()
:   QObject(0), tcpServer(0), networkSession(0)
{
    port = 0;
    ipAddress = "";
    tcpServer = 0;
    networkSession = 0;
    sockets = new QMap<QString, QTcpSocket *>();
}

Server::~Server()
{
    if (networkSession) delete networkSession;
    if (tcpServer) delete tcpServer;
    delete sockets;
}

void Server::StartServer(int server_port)
{
    port = server_port;
    emit write_message(tr("Network session starting."));

    QNetworkConfigurationManager manager;
    if (manager.capabilities() & QNetworkConfigurationManager::NetworkSessionRequired) {
        // Get saved network configuration
        QSettings settings(QSettings::UserScope, QLatin1String("QtProject"));
        settings.beginGroup(QLatin1String("QtNetwork"));
        const QString id = settings.value(QLatin1String("DefaultNetworkConfiguration")).toString();
        settings.endGroup();

        // If the saved network configuration is not currently discovered use the system default
        QNetworkConfiguration config = manager.configurationFromIdentifier(id);
        if ((config.state() & QNetworkConfiguration::Discovered) !=
            QNetworkConfiguration::Discovered) {
            config = manager.defaultConfiguration();
        }

        networkSession = new QNetworkSession(config, this);
        connect(networkSession, SIGNAL(opened()), this, SLOT(sessionOpened()));

        emit write_message(tr("Opening network session."));
        networkSession->open();
    } else {
        sessionOpened();
    }

    connect(tcpServer, SIGNAL(newConnection()), this, SLOT(sendData()));
}

void Server::sessionOpened()
{
    emit write_message(tr("Network session opened."));

    // Save the used configuration
    if (networkSession) {
        QNetworkConfiguration config = networkSession->configuration();
        QString id;
        if (config.type() == QNetworkConfiguration::UserChoice)
            id = networkSession->sessionProperty(QLatin1String("UserChoiceConfiguration")).toString();
        else
            id = config.identifier();

        QSettings settings(QSettings::UserScope, QLatin1String("QtProject"));
        settings.beginGroup(QLatin1String("QtNetwork"));
        settings.setValue(QLatin1String("DefaultNetworkConfiguration"), id);
        settings.endGroup();
    }

    tcpServer = new QTcpServer(this);
    if (!tcpServer->listen(QHostAddress::Any, port)) {
        emit error(tr("Unable to start the server: %1.").arg(tcpServer->errorString()));
        return;
    }

    QList<QHostAddress> ipAddressesList = QNetworkInterface::allAddresses();
    // use the first non-localhost IPv4 address
    for (int i = 0; i < ipAddressesList.size(); ++i) {
        if (ipAddressesList.at(i) != QHostAddress::LocalHost &&
            ipAddressesList.at(i).toIPv4Address()) {
            ipAddress = ipAddressesList.at(i).toString();
            break;
        }
    }
    // if we did not find one, use IPv4 localhost
    if (ipAddress.isEmpty())
        ipAddress = QHostAddress(QHostAddress::LocalHost).toString();

    emit write_message(tr("The server is running on\n\nIP: %1\nport: %2\n").arg(ipAddress).arg(tcpServer->serverPort()));
}

void Server::sendData()
{
    QByteArray block;

    QTcpSocket *clientConnection = tcpServer->nextPendingConnection();
    connect(clientConnection, SIGNAL(disconnected()), clientConnection, SLOT(deleteLater()));

    QString client_ip = clientConnection->peerAddress().toString();

    emit write_message(tr("New connection from IP: %1").arg(client_ip));

    if (sockets->contains(client_ip)) {
        ((QTcpSocket*) sockets->value(client_ip))->disconnectFromHost();
        sockets->remove(client_ip);
    }

    sockets->insert(client_ip, clientConnection);

    clientConnection->write(block);

    connect(clientConnection, SIGNAL(readyRead()), this, SLOT(recieveData()));
    connect(clientConnection, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(displayError(QAbstractSocket::SocketError)));
}

void Server::displayError(QAbstractSocket::SocketError socketError)
{
    switch (socketError) {
        case QAbstractSocket::RemoteHostClosedError:
            break;
        case QAbstractSocket::HostNotFoundError:
            emit write_message(tr("The host was not found. Please check the host name and port settings."));
            break;
        case QAbstractSocket::ConnectionRefusedError:
            emit write_message(tr("The connection was refused by the peer. Make sure the fortune server is running, and check that the host name and port settings are correct."));
            break;
        default:
            emit write_message(tr("Error"));
    }
}

void Server::recieveData()
{
    QMap<QString, QTcpSocket *>::const_iterator i = sockets->constBegin();

    while (i != sockets->constEnd()) {
        QTcpSocket* tcpSocket = i.value();
        if (tcpSocket && tcpSocket->bytesAvailable()>0) {
            int size = tcpSocket->bytesAvailable();
            QDataStream in(tcpSocket);
            in.setVersion(QDataStream::Qt_4_0);
            char *mem = new char[size];
            in.readRawData(mem, size);
            QString message = QString::fromLatin1(mem, size);
            delete mem;

            qDebug() << tcpSocket->peerAddress().toString() << " " << size << " " << message << endl;
        }
        ++i;
    }
}

