#include "server.h"

Server::Server()
:   AbstractServer(0), tcpServer(0), networkSession(0)
{
    port = 0;
    remote_port = 0;
    ipAddress = "";
    tcpServer = 0;
    networkSession = 0;
    sockets = new QMap<quint32, QTcpSocket *>();
    clientblocks = new QMap<quint16, ClientBlock *>();
}

Server::~Server()
{
    StopServer();
    delete clientblocks;
    delete sockets;
}

void Server::Reset()
{
    emit write_message(tr("Reseting server."));

    if (!sockets->isEmpty()) {
        QMap<quint32, QTcpSocket *>::const_iterator i = sockets->constBegin();
        while (i != sockets->constEnd()) {
            QTcpSocket* tcpSocket = i.value();
            if (tcpSocket) {
                emit write_message(tr("Sending reset signal to %1.").arg(tcpSocket->peerAddress().toString()));
                tcpSocket->write("SERV_RST=1\r\n\r\n");
                tcpSocket->flush();
            }
            ++i;
        }
        sockets->clear();
    }
    if (tcpServer) {
        tcpServer->close();
    }
    sessionOpened();
}

bool Server::isStarted()
{
    return !!networkSession;
}

void Server::StartServer()
{
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

void Server::StopServer()
{
    QMap<quint16, ClientBlock *>::const_iterator i = clientblocks->constBegin();
    while (i != clientblocks->constEnd()) {
        delete i.value();
        ++i;
    }
    clientblocks->clear();
    sockets->clear();
    if (tcpServer) {
        if (tcpServer->isListening()) {
            tcpServer->close();
        }
        delete tcpServer;
        tcpServer = NULL;
    }
    if (networkSession) {
        if (networkSession->isOpen()) {
            networkSession->close();
        }
        delete networkSession;
        networkSession = NULL;
    }
}

bool Server::SendData(QString ip_to, QString message)
{
    ip_to = ip_to.replace(QRegExp("(\\s){0,}\\(.*\\)(\\s){0,}"), "");
    return SendData(QHostAddress(ip_to), message);
}

bool Server::SendData(QHostAddress ip_to, QString message)
{
    bool result = false;
    QTcpSocket* tcpSocket = sockets->value(ip_to.toIPv4Address(), 0);
    if (tcpSocket && tcpSocket->state()==QAbstractSocket::ConnectedState && tcpSocket->isWritable()) {
        emit write_message(tr("Sending data (size=%1) to %2. Content: \"%3\"").arg(message.length()).arg(ip_to.toString()).arg(message));
        tcpSocket->write(message.toUtf8());
        result = tcpSocket->waitForBytesWritten();
    } else {
        tcpSocket = new QTcpSocket();
        tcpSocket->connectToHost(ip_to, remote_port, QIODevice::ReadWrite);
        tcpSocket->waitForConnected(5000);

        if (tcpSocket->state()==QAbstractSocket::ConnectedState) {
            emit write_message(tr("Sending data (size=%1) from new socket to %2. Content: \"%3\"").arg(message.length()).arg(ip_to.toString()).arg(message));
            tcpSocket->write(message.toUtf8());
            result = tcpSocket->waitForBytesWritten(5000);
        } else {
            emit error(tr("Client \"%1\" not found").arg(ip_to.toString()));
        }

        tcpSocket->abort();
        delete tcpSocket;
    }
    return result;
}

bool Server::SendData(quint16 block_id, QString message)
{
    bool result = false;
    ClientBlock *bl = clientblocks->value(block_id, NULL);
    if (bl) {
        result = SendData(QHostAddress(bl->getIpAddr()), message);
    }
    return result;
}

bool Server::SendSetConfigsAndReset(QString ip_to, QString ssid, QString pssw, QString servip, quint8 stid, quint8 lcd_i2c_addr)
{
    QString command = "DS_SETUP:\r\n"+ssid+"\r\n"+pssw+"\r\n"+servip+"\r\n"+QString::number(stid)+"\r\n"+QString::number(lcd_i2c_addr)+"\r\n";
    return SendData(ip_to, command);
}

quint16 Server::getNextBlockID(quint16 block_id)
{
    if (clientblocks->isEmpty()) {
        return 0;
    }

    QMap<quint16, ClientBlock *>::const_iterator i = clientblocks->find(block_id);

    if (i != clientblocks->constEnd()) {
        i++;
        if (i != clientblocks->constEnd()) {
            return i.key();
        }
    }

    return clientblocks->firstKey();
}

quint16 Server::getFirstBlockID()
{
    if (clientblocks->isEmpty()) {
        return 0;
    }
    return clientblocks->firstKey();
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

void Server::clientBlockSensorsChange()
{
    ClientBlock *cblock = dynamic_cast<ClientBlock*>(this->sender());
    if (cblock) {
        emit sensors_change(cblock->getblockId());
    }
}

void Server::clientBlockNewSensor(Sensor* sensor_obj)
{
    emit new_sensor(sensor_obj);
}

void Server::clientBlockReady()
{
    ClientBlock *cblock = dynamic_cast<ClientBlock*>(this->sender());
    if (cblock) {
        emit new_block_ready(cblock->getblockId());
    }
}

void Server::setTimeEvtTo(int value)
{
    time_evt_to = value;
}

void Server::setTimeEvtFrom(int value)
{
    time_evt_from = value;
}

QString Server::getTranslit(QString str)
{
    QString result;
    int i, rU, rL;
    QString validChars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz1234567890-_,. ()[]{}<>~!@#$%^&*+=?";
    QString rusUpper = "АБВГДЕЁЖЗИЙКЛМНОПРСТУФХЦЧШЩЫЭЮЯ";
    QString rusLower = "абвгдеёжзийклмнопрстуфхцчшщыэюя";
    QStringList latUpper, latLower;
    latUpper <<"A"<<"B"<<"V"<<"G"<<"D"<<"E"<<"Jo"<<"Zh"<<"Z"<<"I"<<"J"<<"K"<<"L"<<"M"<<"N"
        <<"O"<<"P"<<"R"<<"S"<<"T"<<"U"<<"F"<<"H"<<"C"<<"Ch"<<"Sh"<<"Sh"<<"I"<<"E"<<"Ju"<<"Ja";
    latLower <<"a"<<"b"<<"v"<<"g"<<"d"<<"e"<<"jo"<<"zh"<<"z"<<"i"<<"j"<<"k"<<"l"<<"m"<<"n"
        <<"o"<<"p"<<"r"<<"s"<<"t"<<"u"<<"f"<<"h"<<"c"<<"ch"<<"sh"<<"sh"<<"i"<<"e"<<"ju"<<"ja";
    for (i=0; i < str.size(); ++i){
        if ( validChars.contains(str[i]) ){
            result = result + str[i];
        }else{
            rU = rusUpper.indexOf(str[i]);
            rL = rusLower.indexOf(str[i]);
            if (rU >= 0)         result = result + latUpper[rU];
            else if (rL >= 0)    result = result + latLower[rL];
        }
    }
    return result;
}

void Server::processEventGroup(QString message_type, QString message, QTcpSocket *tcpSocket)
{
    QString smsg;
    int idusersocs=tcpSocket->socketDescriptor();
    QTextStream os(tcpSocket);
    os.setAutoDetectUnicode(true);
    os << "HTTP/1.0 200 Ok\r\n"
          "Content-Type: text/html; charset=\"utf-8\"\r\n"
          "\r\n"
          "<h1>Nothing to see here</h1>\n"
          << QDateTime::currentDateTime().toString() << "\n";

    if (message_type == "smsread" || message_type == "phoneread") {

        QMap<quint16, ClientBlock *>::const_iterator i = clientblocks->constBegin();
        while (i != clientblocks->constEnd()) {
            smsg  = "TONE=0\n";
            smsg += "SERV_LT=";
            SendData(i.value()->getblockId(), smsg);
            ++i;
        }

    } else if (message_type == "sms" || message_type == "phone" || message_type == "notify") {

        QString username = "";
        int index = message.indexOf("INFO=");
        if (index >= 0) {
            index += 5;
            username = message.mid(index);
            QUrl upath = QUrl(username);
            username = upath.fromPercentEncoding(username.toUtf8());
            username = getTranslit(username);
            emit write_message(username);
        }
        username = username.replace('+', " ");
        if (username.length() > 16) {
            username.resize(16);
        }

        bool need_sound = false;
        int hour = QDateTime::currentDateTime().toString("hh").toInt();
        if (time_evt_to >= time_evt_from) {
            if (hour <= time_evt_from || hour >= time_evt_to) {
                need_sound = true;
            }
        } else {
            if (hour >= time_evt_to && hour <= time_evt_from) {
                need_sound = true;
            }
        }

        QMap<quint16, ClientBlock *>::const_iterator i = clientblocks->constBegin();
        while (i != clientblocks->constEnd()) {
            smsg = "";
            if (message_type == "sms") {
                if (need_sound) smsg += "TONE=300,1500\n";
                smsg += "SERV_LT=SMS  " + QDateTime::currentDateTime().toString("dd.MM hh:mm") + username;
            } else if (message_type == "phone") {
                if (need_sound) smsg += "TONE=500,500\n";
                smsg += "SERV_LT=CALL " + QDateTime::currentDateTime().toString("dd.MM hh:mm") + username;
            } else {
                if (need_sound) smsg += "TONE=800,2000\n";
                smsg += "SERV_LT=NOT  " + QDateTime::currentDateTime().toString("dd.MM hh:mm") + username;
            }
            SendData(i.value()->getblockId(), smsg);
            ++i;
        }

    }
}

int Server::getPort() const
{
    return port;
}

void Server::setPort(int value)
{
    port = value;
}

int Server::getRemotePort() const
{
    return remote_port;
}

void Server::setRemotePort(int value)
{
    remote_port = value;
}

ClientBlock *Server::getClientBlock(QString ip_addr)
{
    ip_addr = ip_addr.replace(QRegExp("(\\s){0,}\\(.*\\)(\\s){0,}"), "");
    return getClientBlock(QHostAddress(ip_addr).toIPv4Address());
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

ClientBlock *Server::getClientBlockByID(quint16 block_id)
{
    if (clientblocks->contains(block_id)) {
        return clientblocks->value(block_id);
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
            QString message = QString::fromUtf8(mem, size).trimmed();
            delete mem;
            emit write_message(tr("Recieved data (size=%1) from %2. Content: \"%3\"").arg(size).arg(tcpSocket->peerAddress().toString()).arg(message));

            if (message.startsWith("POST /sms HTTP/")) {
                processEventGroup("sms", message, tcpSocket);
            } else if (message.startsWith("POST /smsread HTTP/")) {
                processEventGroup("smsread", message, tcpSocket);
            } else if (message.startsWith("POST /phone HTTP/")) {
                processEventGroup("phone", message, tcpSocket);
            } else if (message.startsWith("POST /phoneread HTTP/")) {
                processEventGroup("phoneread", message, tcpSocket);
            } else if (message.startsWith("POST /notify HTTP/")) {
                processEventGroup("notify", message, tcpSocket);
            } else  {
                processMessageGroup(message, i.key());
            }
        }
        ++i;
    }
}

void Server::processMessageGroup(QString message_gr, quint32 ipaddr)
{
    QStringList message_list = message_gr.split("\r\n", QString::KeepEmptyParts);
    for(int i=0; i<message_list.size(); i++) {
        processMessage(message_list.at(i), ipaddr);
    }
}

void Server::processMessage(QString message, quint32 ipaddr)
{
    if (message.length()>3 && message.length()<=5 && message.startsWith("DS=")) {
        int dst_id = message.remove(0,3).toInt();
        if (dst_id) {
            if (clientblocks->contains(dst_id)) {
                clientblocks->value(dst_id)->reset();
                clientblocks->value(dst_id)->setIpAddr(ipaddr);
                emit write_message(tr("Block with ID=%1 connected.").arg(dst_id));
            } else {
                ClientBlock *nblock = new ClientBlock(this, dst_id);
                nblock->setIpAddr(ipaddr);
                clientblocks->insert(dst_id, nblock);
                connect(nblock, SIGNAL(sensors_values_changed()), this, SLOT(clientBlockSensorsChange()));
                connect(nblock, SIGNAL(new_sensor(Sensor*)), this, SLOT(clientBlockNewSensor(Sensor*)));
                connect(nblock, SIGNAL(block_ready()), this, SLOT(clientBlockReady()));
                emit write_message(tr("Registered new block. ID=%1").arg(dst_id));
            }
            emit blocks_change();
        }
    } else {
        ClientBlock *rblock = getClientBlock(ipaddr);
        if (rblock) {
            rblock->BlockMessage(message);
        }
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
