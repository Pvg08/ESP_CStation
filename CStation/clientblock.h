#ifndef CLIENTBLOCK_H
#define CLIENTBLOCK_H

#include <QObject>
#include <QMap>
#include <QTextStream>
#include "sensor.h"
#include "clientaction.h"

typedef QMap<char, Sensor *> ClientSensors;
typedef QMap<QString, ClientAction *> ClientActions;

class ClientBlock : public QObject
{
    Q_OBJECT
public:
    explicit ClientBlock(QObject *parent, quint16 id);
    ~ClientBlock();
    quint32 getIpAddr() const;
    void setIpAddr(const quint32 &value);
    ClientSensors *getSensors() const;
    ClientActions *getClientActions() const;

    ClientAction* getAction(QString actionCode);
    Sensor* getSensor(char sensorLetter);
    QString getSensorValue(char sensorLetter);

    void BlockMessage(QString message);
    quint16 getblockId() const;
private:
    quint32 ip_addr;
    quint16 block_id;
    bool is_on;
    ClientSensors *sensors;
    ClientActions *client_actions;
signals:
    void sensors_values_changed();
    void new_sensor(Sensor* new_sensor);
    void new_action(ClientAction* new_action);
private slots:
    void sensor_local_change();
protected:
    void setSensorsValues(QString message);
    void addSensor(QString message);
    void addAction(QString message);
};

#endif // CLIENTBLOCK_H
