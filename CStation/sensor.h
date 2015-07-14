#ifndef SENSOR_H
#define SENSOR_H

#include <QObject>

class Sensor : public QObject
{
    Q_OBJECT
public:
    enum SensorTypes {
        ST_TEMPERATURE,
        ST_PRESSURE,
        ST_WETNESS,
        ST_BRIGHTNESS,
        ST_SOUND,
        ST_PRESENCE
    };
    explicit Sensor(QObject *parent, SensorTypes sensor_type);
    QString getValue() const;
    void setValue(const QString &value);

    Sensor::SensorTypes getSensorType() const;

private:
    SensorTypes SensorType;
    QString sensor_value;
signals:

public slots:
};

#endif // SENSOR_H
