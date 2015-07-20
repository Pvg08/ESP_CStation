#ifndef CLIENTBLOCK_H
#define CLIENTBLOCK_H

#include <QObject>
#include <QMap>
#include <QTextStream>
#include "sensor.h"

class ClientBlock : public QObject
{
    Q_OBJECT
public:
    explicit ClientBlock(QObject *parent, quint16 id);
    ~ClientBlock();
    quint32 getIpAddr() const;
    void setIpAddr(const quint32 &value);
    QMap<char, Sensor *> *getSensors() const;
    Sensor* getSensor(char sensorLetter);
    QString getSensorValue(char sensorLetter);
    void BlockMessage(QString message);
    quint16 getblockId() const;
private:
    quint32 ip_addr;
    quint16 block_id;
    bool is_on;
    QMap<char, Sensor *> *sensors;
signals:
    void sensors_values_changed();
private slots:
    void sensor_local_change();
protected:
    void setSensorsValues(QString message);
    void addSensor(QString message);
};

#endif // CLIENTBLOCK_H
