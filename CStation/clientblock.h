#ifndef CLIENTBLOCK_H
#define CLIENTBLOCK_H

#include <QObject>
#include <QMap>
#include "sensor.h"

class ClientBlock : public QObject
{
    Q_OBJECT
public:
    explicit ClientBlock(QObject *parent, quint16 id);
    ~ClientBlock();
    void WriteToLedMatrix(QByteArray data);
    void WriteToLCD(QString string);
    quint32 getIpAddr() const;
    void setIpAddr(const quint32 &value);
    QString getSensorValue(Sensor::SensorTypes sensor_type);
    void BlockMessage(QString message);
    quint16 getblockId() const;

private:
    quint32 ip_addr;
    quint16 block_id;
    bool is_on;
    QMap<Sensor::SensorTypes, Sensor *> *sensors;
signals:
    void PeriodicSensorData();
    void FastSensorData();
public slots:

};

#endif // CLIENTBLOCK_H
