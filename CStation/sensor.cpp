#include "sensor.h"

Sensor::Sensor(QObject *parent, SensorTypes sensor_type) : QObject(parent)
{
    SensorType = sensor_type;
    sensor_value = "";
}

QString Sensor::getValue() const
{
    return sensor_value;
}

void Sensor::setValue(const QString &value)
{
    sensor_value = value;
}

Sensor::SensorTypes Sensor::getSensorType() const
{
    return SensorType;
}



