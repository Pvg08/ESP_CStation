#include "clientblock.h"

ClientBlock::ClientBlock(QObject *parent, quint16 id) : QObject(parent)
{
    ip_addr = 0;
    block_id = id;
    is_on = false;
    sensors = new QMap<Sensor::SensorTypes, Sensor *>();
}

ClientBlock::~ClientBlock()
{
    QMap<Sensor::SensorTypes, Sensor *>::const_iterator i = sensors->constBegin();
    while (i != sensors->constEnd()) {
        delete i.value();
        ++i;
    }
    delete sensors;
}

void ClientBlock::WriteToLedMatrix(QByteArray data)
{

}

void ClientBlock::WriteToLCD(QString string)
{

}

quint32 ClientBlock::getIpAddr() const
{
    return ip_addr;
}

void ClientBlock::setIpAddr(const quint32 &value)
{
    ip_addr = value;
}

QString ClientBlock::getSensorValue(Sensor::SensorTypes sensor_type)
{
    if (sensors->contains(sensor_type)) {
        return sensors->value(sensor_type)->getValue();
    } else {
        return "";
    }
}

void ClientBlock::BlockMessage(QString message)
{

}
quint16 ClientBlock::getblockId() const
{
    return block_id;
}


