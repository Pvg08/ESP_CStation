#ifndef SENSOR_H
#define SENSOR_H

#include <QObject>
#include <QTextStream>
#include <QDateTime>
#include <QTimer>

class Sensor : public QObject
{
    Q_OBJECT
public:
    enum SensorDataTypes {
        SDT_FLOAT,
        SDT_INT,
        SDT_ENUM
    };
    explicit Sensor(QObject *parent, QString sensor_description);
    ~Sensor();

    QString getValue();
    QString getValueWithEM();
    float getFloatValue();
    void setValue(const QString &value);

    QString getSensorEM() const;
    quint16 getResetTime() const;
    QString getSensorName() const;
    char getSensorLetter() const;
    float getFromValue() const;
    float getToValue() const;
    QString getEnumTrue() const;
    QString getEnumFalse() const;
    Sensor::SensorDataTypes getSensorDataType() const;
    QString getSensorDescription() const;
    void setSensorDescription(const QString &value);
    bool getIsValid() const;
    QDateTime getUpdTime() const;

private:
    QString sensorDescription;
    QString sensorValue;

    QTimer *shotTimer;

    char sensorLetter;
    float fromValue, toValue;
    QString enumTrue, enumFalse;
    SensorDataTypes sensorDataType;
    QString sensorName;
    QString sensorEM;
    quint16 resetTime;

    QDateTime upd_time;

    bool isValid;

    void parseDescription();
private slots:
    void reset_value();
signals:
    void local_change();

public slots:
};

#endif // SENSOR_H
