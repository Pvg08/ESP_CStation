#ifndef SENSOR_H
#define SENSOR_H

#include <QObject>
#include <QCoreApplication>
#include <QTextStream>
#include <QDateTime>
#include <QTimer>
#include <QFile>
#include <QDir>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonParseError>
#include <math.h>

#define SENSORS_LOG_BUFFER_FLUSH_SIZE 128
#define SENSORS_LOG_BUFFER_PRECISION 1E-08

#if defined(WIN32) || defined(__WATCOMC__) || defined(_WIN32) || defined(__WIN32__)
    #define __PACKED
#else
    #define __PACKED __attribute__((packed)) /* gcc packed */
#endif

#if defined(WIN32) || defined(_WIN64) || defined(__WATCOMC__) || defined(_WIN32) || defined(__WIN32__)
#pragma pack(push, 1)
#endif

typedef struct
{
    qint64 log_time __PACKED;
    float log_value __PACKED;
} SensorLogItem __PACKED;

#if defined(WIN32) || defined(_WIN64) || defined(__WATCOMC__) || defined(_WIN32) || defined(__WIN32__)
#pragma pack(pop)
#endif


class Sensor : public QObject
{
    Q_OBJECT
public:

    enum SensorDataTypes {
        SDT_FLOAT,
        SDT_INT,
        SDT_ENUM_BOOL
    };
    explicit Sensor(QObject *parent, QString sensor_description);
    ~Sensor();

    QString getValue();
    QString getValueWithEM();
    float getFloatValue();
    bool valueIsCorrect();
    void setValue(const QString &value);

    QString getSensorEM() const;
    quint16 getResetTime() const;
    QString getSensorName() const;
    QString getSensorCode() const;
    float getFromValue() const;
    float getToValue() const;
    QString getEnumTrue() const;
    QString getEnumFalse() const;
    QString getTrEnumTrue() const;
    QString getTrEnumFalse() const;
    Sensor::SensorDataTypes getSensorDataType() const;
    QString getSensorDescription() const;
    void setSensorDescription(const QString &value);
    bool getIsValid() const;
    QDateTime getUpdTime() const;

    quint16 getBlockID() const;
    void setBlockID(const quint16 &value);

    QList<SensorLogItem>* startLogDataTracking(quint64 time_sub);
    void stopLogDataTracking();
    qint64 getLogBufferTimeSub() const;

    QList<SensorLogItem> *getLogBuffer();

private:
    QFile *log_file;
    QList<SensorLogItem>* log_buffer;
    qint64 log_buffer_time_sub;
    SensorLogItem last_log_item;

    QString sensorDescription;
    QString sensorValue;

    QTimer *shotTimer;

    QString sensorCode;
    float fromValue, toValue;
    QString enumTrue, enumFalse;
    SensorDataTypes sensorDataType;
    QString sensorName;
    QString sensorEM;
    quint16 resetTime;
    quint16 block_id;

    QDateTime upd_time;

    bool isValid;
    bool skip_enum_check;
    bool buffer_is_loading;

    void parseDescription();
    bool writeLog(bool check_precision);
    bool initLogFile();
    void cutLogBuffer();
    QString html_decode(const QString &str);
private slots:
    void reset_value();
signals:
    void local_change();
    void value_change();

public slots:
};

#endif // SENSOR_H
