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

Sensor *ClientBlock::getSensor(char sensorLetter)
{
    return sensors->value(sensorLetter, NULL);
}

QString ClientBlock::getSensorValue(char sensorLetter)
{
    if (sensors->contains(sensorLetter)) {
        return sensors->value(sensorLetter)->getValue();
    } else {
        return "";
    }
}

void ClientBlock::BlockMessage(QString message)
{
    if (message.length()>9 && message.startsWith("DS_INFO=")) {
        message = message.remove(0,8);
        addSensor(message);
    } else if (message.length()>9 && message.startsWith("DC_INFO=")) {
        message = message.remove(0,8);
        addAction(message);
    } else if (message.length()>7 && message.startsWith("DS_V={")) {
        message = message.remove(0,6);
        message.truncate(message.length()-1);
        setSensorsValues(message);
    } else if (message.length()>9 && message.startsWith("DS_READY=1")) {
        is_ready = true;
        emit block_ready();
    }
}

void ClientBlock::setSensorsValues(QString message)
{
    QStringList sensor_values = message.split(';', QString::SkipEmptyParts);
    for(int i=0; i<sensor_values.length(); i++) {
        QString sensor_msg = sensor_values.at(i);
        if (sensor_msg.length()>=3 && sensor_msg.at(0)>='A' && sensor_msg.at(0)<='Z') {
            char sensor_letter = sensor_msg.at(0).toLatin1();
            if (sensors->contains(sensor_letter)) {
                sensor_msg.replace(QRegExp("^([^\\(\\)]*)\\(([^\\(\\)]*)\\).*$"), "\\2");
                sensors->value(sensor_letter)->setValue(sensor_msg);
            }
        }
    }
    emit sensors_values_changed();
}

void ClientBlock::addSensor(QString message)
{
    Sensor* n_sensor = new Sensor(this, message);
    if (n_sensor->getIsValid() && !sensors->contains(n_sensor->getSensorLetter())) {
        n_sensor->setBlockID(block_id);
        connect(n_sensor, SIGNAL(local_change()), this, SLOT(sensor_local_change()));
        sensors->insert(n_sensor->getSensorLetter(), n_sensor);
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

