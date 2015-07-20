#include "server.h"

Server::Server()
:   QObject(0), tcpServer(0), networkSession(0)
{
    port = 0;
    ipAddress = "";
    tcpServer = 0;
    networkSession = 0;
    sockets = new QMap<quint32, QTcpSocket *>();
    clientblocks = new QMap<quint16, ClientBlock *>();
}

Server::~Server()
{
    QMap<quint16, ClientBlock *>::const_iterator i = clientblocks->constBegin();
    while (i != clientblocks->constEnd()) {
        delete i.value();
        ++i;
    }
    clientblocks->clear();

    if (tcpServer) delete tcpServer;
    if (networkSession) delete networkSession;
    delete clientblocks;
    delete sockets;
}

void Server::Reset(int server_port)
{
    emit write_message(tr("Reseting server."));
    if (!sockets->isEmpty()) {
        QMap<quint32, QTcpSocket *>::const_iterator i = sockets->constBegin();
        while (i != sockets->constEnd()) {
            QTcpSocket* tcpSocket = i.value();
            if (tcpSocket) {
                emit write_message(tr("Sending reset signal to %1.").arg(tcpSocket->peerAddress().toString()));
                tcpSocket->write("RST\r\n\r\n");
                tcpSocket->flush();
                tcpSocket->disconnectFromHost();
            }
            ++i;
        }
        sockets->clear();
    }
    if (tcpServer) {
        tcpServer->close();
    }
    if (server_port) port = server_port;
    sessionOpened();
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

    connect(tcpServer, SIGNAL(newConnection()), this, SLOT(recieveConnection()));
}

bool Server::SendData(QString ip_to, QString message)
{
    bool result = false;
    ip_to = ip_to.replace(QRegExp("(\\s){0,}\\(.*\\)(\\s){0,}"), "");
    QTcpSocket* tcpSocket = sockets->value(QHostAddress(ip_to).toIPv4Address(), 0);
    if (tcpSocket && tcpSocket->state()==QAbstractSocket::ConnectedState && tcpSocket->isWritable()) {
        emit write_message(tr("Sending data (size=%1) to %2. Content: \"%3\"").arg(message.length()).arg(ip_to).arg(message));
        tcpSocket->write(message.toLocal8Bit());
        result = tcpSocket->waitForBytesWritten();
    } else {
        tcpSocket = new QTcpSocket();
        tcpSocket->connectToHost(QHostAddress(ip_to), port, QIODevice::ReadWrite);
        tcpSocket->waitForConnected(5000);

        if (tcpSocket->state()==QAbstractSocket::ConnectedState) {
            emit write_message(tr("Sending data (size=%1) from new socket to %2. Content: \"%3\"").arg(message.length()).arg(ip_to).arg(message));
            tcpSocket->write(message.toLocal8Bit());
            result = tcpSocket->waitForBytesWritten(5000);
        } else {
            emit error(tr("Client \"%1\" not found").arg(ip_to));
        }

        tcpSocket->abort();
        delete tcpSocket;
    }
    return result;
}

bool Server::SendReboot(QString ip_to)
{
    return SendData(ip_to, "SERV_RST=1\r\n");
}

bool Server::SendRunSetup(QString ip_to)
{
    return SendData(ip_to, "SERV_CONF=1\r\n");
}

bool Server::SendSetConfigsAndReset(QString ip_to, QString ssid, QString pssw, QString servip, quint8 stid)
{
    QString command = "DS_SETUP:\r\n"+ssid+"\r\n"+pssw+"\r\n"+servip+"\r\n"+QString::number(stid)+"\r\n";
    return SendData(ip_to, command);
}

bool Server::SendTone(QString ip_to, unsigned frequency)
{
    return SendData(ip_to, "TONE="+QString::number(frequency)+"\r\n");
}

const QStringList Server::getIPsList()
{
    QStringList result;
    QMap<quint16, ClientBlock *>::const_iterator i = clientblocks->constBegin();
    while (i != clientblocks->constEnd()) {
        result.append(QHostAddress(i.value()->getIpAddr()).toString()+" (DS"+QString::number(i.value()->getblockId())+")");
        ++i;
    }
    return result;
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

    if (!tcpServer) tcpServer = new QTcpServer(this);
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

    emit write_message(tr("The server is running on IP: %1 port: %2\n").arg(ipAddress).arg(tcpServer->serverPort()));
}

void Server::recieveConnection()
{
    QTcpSocket *clientConnection = tcpServer->nextPendingConnection();
    connect(clientConnection, SIGNAL(disconnected()), clientConnection, SLOT(deleteLater()));

    QString client_ip = clientConnection->peerAddress().toString();
    quint32 client_ip_int = clientConnection->peerAddress().toIPv4Address();

    emit write_message(tr("New connection from IP: %1").arg(client_ip));

    if (sockets->contains(client_ip_int)) {
        QTcpSocket *oldClientConnection = (QTcpSocket*) sockets->value(client_ip_int);
        if (oldClientConnection && oldClientConnection->state() != QAbstractSocket::UnconnectedState) {
            oldClientConnection->disconnectFromHost();
        }
        sockets->remove(client_ip_int);
    }

    sockets->insert(client_ip_int, clientConnection);

    connect(clientConnection, SIGNAL(stateChanged(QAbstractSocket::SocketState)), this, SLOT(socketStateChanged(QAbstractSocket::SocketState)));
    connect(clientConnection, SIGNAL(disconnected()), this, SLOT(clientDisconnected()));
    connect(clientConnection, SIGNAL(readyRead()), this, SLOT(recieveData()));
    connect(clientConnection, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(displayError(QAbstractSocket::SocketError)));
}

void Server::displayError(QAbstractSocket::SocketError socketError)
{
    QTcpSocket *clientConnection = dynamic_cast<QTcpSocket*>(this->sender());
    if (clientConnection != NULL) {
        switch (socketError) {
            case QAbstractSocket::RemoteHostClosedError:
                break;
            case QAbstractSocket::HostNotFoundError:
                emit write_message(tr("The host was not found. Please check the host name settings."));
                break;
            case QAbstractSocket::ConnectionRefusedError:
                emit write_message(tr("The connection was refused by the peer %1. Make sure the client is running, and check that the host name is correct.").arg(clientConnection->peerAddress().toString()));
                break;
            default:
                emit write_message(tr("Error"));
        }
    }
}

ClientBlock *Server::getClientBlock(quint32 ip_addr)
{
    QMap<quint16, ClientBlock *>::const_iterator i = clientblocks->constBegin();
    while (i != clientblocks->constEnd()) {
        if (i.value()->getIpAddr()==ip_addr) {
            return i.value();
        }
        ++i;
    }
    return NULL;
}

void Server::recieveData()
{
    QMap<quint32, QTcpSocket *>::const_iterator i = sockets->constBegin();

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
            emit write_message(tr("Recieved data (size=%1) from %2. Content: \"%3\"").arg(size).arg(tcpSocket->peerAddress().toString()).arg(message));

            if (message.length()>3 && message.length()<=5 && message.startsWith("DS=")) {
                int dst_id = message.remove(0,3).toInt();
                if (dst_id) {
                    if (clientblocks->contains(dst_id)) {
                        clientblocks->value(dst_id)->setIpAddr(i.key());
                        emit write_message(tr("Block with ID=%1 connected.").arg(dst_id));
                    } else {
                        ClientBlock *nblock = new ClientBlock(this, dst_id);
                        nblock->setIpAddr(i.key());
                        clientblocks->insert(dst_id, nblock);
                        emit write_message(tr("Registered new block. ID=%1").arg(dst_id));
                    }
                    emit blocks_change();
                }
            } else {
                ClientBlock *rblock = getClientBlock(i.key());
                if (rblock) {
                    rblock->BlockMessage(message);
                }
            }

        }
        ++i;
    }
}

void Server::clientDisconnected()
{
    QTcpSocket *clientConnection = dynamic_cast<QTcpSocket*>(this->sender());
    if (clientConnection) {
        QMap<quint32, QTcpSocket *>::const_iterator i = sockets->constBegin();
        while (i != sockets->constEnd()) {
            if (clientConnection == i.value()) {
                sockets->remove(i.key());
                emit write_message(tr("Socket removed."));
                break;
            }
            ++i;
        }
    }
}

void Server::socketStateChanged(QAbstractSocket::SocketState state)
{
    QTcpSocket *clientConnection = dynamic_cast<QTcpSocket*>(this->sender());
    if (clientConnection) {
        QString host_ip = clientConnection->peerAddress().toString();
        switch(state) {
            case QAbstractSocket::HostLookupState:
            emit write_message(tr("%1 is performing a host name lookup.").arg(host_ip));
            break;
            case QAbstractSocket::ConnectingState:
            emit write_message(tr("%1 has started establishing a connection.").arg(host_ip));
            break;
            case QAbstractSocket::ConnectedState:
            emit write_message(tr("%1 established a connection.").arg(host_ip));
            break;
            case QAbstractSocket::BoundState:
            emit write_message(tr("%1 is bound to an address and port.").arg(host_ip));
            break;
            case QAbstractSocket::ClosingState:
            emit write_message(tr("%1 is closing.").arg(host_ip));
            break;
        }
    }
}
