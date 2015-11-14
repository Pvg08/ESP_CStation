#include "clientblock.h"

ClientBlock::ClientBlock(AbstractServer *parent, quint16 id) : QObject(parent)
{
    ip_addr = 0;
    block_id = id;
    is_on = false;
    is_ready = false;
    sensors = new ClientSensors();
    client_actions = new ClientActions();
}

ClientBlock::~ClientBlock()
{
    ClientSensors::const_iterator i = sensors->constBegin();
    while (i != sensors->constEnd()) {
        delete i.value();
        ++i;
    }
    delete sensors;
    delete client_actions;
}

quint32 ClientBlock::getIpAddr() const
{
    return ip_addr;
}

void ClientBlock::setIpAddr(const quint32 &value)
{
    ip_addr = value;
}

ClientAction *ClientBlock::getAction(QString actionCode)
{
    return client_actions->value(actionCode, NULL);
}

Sensor *ClientBlock::getSensor(QString sensorCode)
{
    return sensors->value(sensorCode, NULL);
}

QString ClientBlock::getSensorValue(QString sensorCode)
{
    if (sensors->contains(sensorCode)) {
        return sensors->value(sensorCode)->getValue();
    } else {
        return "";
    }
}

void ClientBlock::BlockMessage(QString message)
{
    QString cmd_name = message;
    cmd_name.replace(QRegExp("=(.*)$"), "");
    if(cmd_name.length()==message.length()) cmd_name="";
    if (cmd_name.isEmpty()) return;

    QString cmd_param = message;
    cmd_param.replace(QRegExp("^([^=]*)=(.*)$"), "\\2");

    if (cmd_name=="DS_INFO") {
        addSensor(cmd_param);
    } else if (cmd_name=="DC_INFO") {
        addAction(cmd_param);
    } else if (cmd_name=="DS_V") {
        setSensorsValues(cmd_param);
    } else if (cmd_name=="DS_READY") {
        if (cmd_param=="1") {
            is_ready = true;
            emit block_ready();
        }
    } else if (cmd_name=="DS_GETTIME") {
        if (cmd_param=="1" && client_actions->contains("settime")) {
            QTimer::singleShot(100, this, SLOT(action_time_send()));
        }
    } else if (cmd_name=="DS_WIFI_SSID") {
        wifi_ssid = cmd_param;
    } else if (cmd_name=="DS_WIFI_PASSW") {
        wifi_passw = cmd_param;
    } else if (cmd_name=="DS_SERVER") {
        ds_server_addr = cmd_param;
    }
}

void ClientBlock::action_time_send()
{
    if (client_actions->contains("settime")) {
        QDateTime dateTime1 = QDateTime::currentDateTime();
        dateTime1.setTimeSpec(Qt::LocalTime);
        quint64 tt = dateTime1.offsetFromUtc() + (QDateTime::currentMSecsSinceEpoch()/1000);
        client_actions->value("settime")->sendCommand(QString::number(tt));
    }
}

void ClientBlock::setSensorsValues(QString message)
{
    message.replace('\'', '"');
    QJsonParseError jerror;
    QJsonDocument jdoc= QJsonDocument::fromJson(message.toLocal8Bit(),&jerror);
    if(jerror.error != QJsonParseError::NoError) return;
    QJsonObject obj = jdoc.object();
    QStringList sensor_keys = obj.keys();
    for(int i=0; i<sensor_keys.length(); i++) {
        QString sensor_code = sensor_keys.at(i);
        QString sensor_value;
        if (obj.value(sensor_code).isString()) {
            sensor_value = obj.value(sensor_code).toString();
        } else {
            sensor_value = QString::number(obj.value(sensor_code).toDouble());
        }
        if (sensors->contains(sensor_code)) {
            sensors->value(sensor_code)->setValue(sensor_value);
        }
    }
    emit sensors_values_changed();
}

void ClientBlock::addSensor(QString message)
{
    Sensor* n_sensor = new Sensor(this, message);
    if (n_sensor->getIsValid() && !sensors->contains(n_sensor->getSensorCode())) {
        n_sensor->setBlockID(block_id);
        connect(n_sensor, SIGNAL(local_change()), this, SLOT(sensor_local_change()));
        sensors->insert(n_sensor->getSensorCode(), n_sensor);
        emit new_sensor(n_sensor);
        emit sensors_values_changed();
    } else {
        delete n_sensor;
    }
}

void ClientBlock::addAction(QString message)
{
    ClientAction* n_action = new ClientAction(dynamic_cast<AbstractServer*>(this->parent()), message);
    if (n_action->actionIsReady() && !client_actions->contains(n_action->getCode())) {
        n_action->setBlockID(block_id);
        client_actions->insert(n_action->getCode(), n_action);
        emit new_action(n_action);
    } else {
        delete n_action;
    }
}

quint16 ClientBlock::getblockId() const
{
    return block_id;
}

bool ClientBlock::isReady() const
{
    return is_ready;
}

void ClientBlock::reset()
{
    is_ready = false;
}

QString ClientBlock::getWifiSSID() const
{
    return wifi_ssid;
}

QString ClientBlock::getWifiPassw() const
{
    return wifi_passw;
}

QString ClientBlock::getDSServerAddr() const
{
    return ds_server_addr;
}

ClientSensors *ClientBlock::getSensors()
{
    return sensors;
}

ClientActions *ClientBlock::getClientActions()
{
    return client_actions;
}

void ClientBlock::sensor_local_change()
{
    emit sensors_values_changed();
}

