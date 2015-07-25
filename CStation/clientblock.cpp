#include "clientblock.h"

ClientBlock::ClientBlock(QObject *parent, quint16 id) : QObject(parent)
{
    ip_addr = 0;
    block_id = id;
    is_on = false;
    sensors = new QMap<char, Sensor *>();
}

ClientBlock::~ClientBlock()
{
    QMap<char, Sensor *>::const_iterator i = sensors->constBegin();
    while (i != sensors->constEnd()) {
        delete i.value();
        ++i;
    }
    delete sensors;
}

quint32 ClientBlock::getIpAddr() const
{
    return ip_addr;
}

void ClientBlock::setIpAddr(const quint32 &value)
{
    ip_addr = value;
}

Sensor *ClientBlock::getSensor(char sensorLetter)
{
    if (sensors->contains(sensorLetter)) {
        return sensors->value(sensorLetter);
    } else {
        return NULL;
    }
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
    } else if (message.length()>7 && message.startsWith("DS_V={")) {
        message = message.remove(0,6);
        message.truncate(message.length()-1);
        setSensorsValues(message);
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
        emit sensors_values_changed();
    } else {
        delete n_sensor;
    }
}

quint16 ClientBlock::getblockId() const
{
    return block_id;
}

QMap<char, Sensor *> *ClientBlock::getSensors() const
{
    return sensors;
}

void ClientBlock::sensor_local_change()
{
    emit sensors_values_changed();
}


