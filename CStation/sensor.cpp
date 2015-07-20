#include "sensor.h"

Sensor::Sensor(QObject *parent, QString sensor_description) : QObject(parent)
{
    sensorValue = "";
    sensorDescription = sensor_description;
    upd_time = QDateTime::currentDateTime();
    shotTimer = new QTimer(this);
    shotTimer->setSingleShot(true);
    connect(shotTimer, SIGNAL(timeout()), SLOT(reset_value()));
    parseDescription();
}

Sensor::~Sensor()
{
    delete shotTimer;
}

QString Sensor::getValue()
{
    return sensorValue;
}

QString Sensor::getValueWithEM()
{
    QString result = getValue();
    if (!sensorEM.isEmpty()) {
        result = result + " " + sensorEM;
    }
    return result;
}

float Sensor::getFloatValue()
{
    float result = 0;
    if (sensorDataType == SDT_ENUM) {
        result = getValue()==enumTrue ? 1 : 0;
    } else {
        result = getValue().toFloat();
    }
    return result;
}

void Sensor::setValue(const QString &value)
{
    upd_time = QDateTime::currentDateTime();
    sensorValue = value;

    if (resetTime && ((sensorDataType == SDT_ENUM && getValue()==enumTrue) || (sensorDataType != SDT_ENUM && getValue().toFloat()!=fromValue))) {
        shotTimer->start();
    }
}

QString Sensor::getSensorEM() const
{
    return sensorEM;
}

quint16 Sensor::getResetTime() const
{
    return resetTime;
}

QString Sensor::getSensorName() const
{
    return sensorName;
}

char Sensor::getSensorLetter() const
{
    return sensorLetter;
}

float Sensor::getFromValue() const
{
    return fromValue;
}

float Sensor::getToValue() const
{
    return toValue;
}

QString Sensor::getEnumTrue() const
{
    return enumTrue;
}

QString Sensor::getEnumFalse() const
{
    return enumFalse;
}

Sensor::SensorDataTypes Sensor::getSensorDataType() const
{
    return sensorDataType;
}

QString Sensor::getSensorDescription() const
{
    return sensorDescription;
}

void Sensor::setSensorDescription(const QString &value)
{
    sensorDescription = value;
    parseDescription();
}

bool Sensor::getIsValid() const
{
    return isValid;
}

QDateTime Sensor::getUpdTime() const
{
    return upd_time;
}

void Sensor::parseDescription()
{
    isValid = false;
    enumTrue = "";
    enumFalse = "";
    sensorDataType = SDT_FLOAT;
    sensorName = "";
    fromValue = 0;
    toValue = 0;
    sensorLetter = 'Z';
    resetTime = 0;
    sensorEM = "";
    if (sensorDescription.length()<10) return;
    sensorLetter = sensorDescription.at(0).toLatin1();

    if (sensorLetter<'A' || sensorLetter>'Z') return;

    QString stype = sensorDescription;
    stype.replace(QRegExp("^([^:]*):([^\\(\\)]*)\\(.*"), "\\2");

    if (stype == "int") {
        sensorDataType = SDT_INT;
    } else if (stype == "enum") {
        sensorDataType = SDT_ENUM;
    } else if (stype != "float") {
        return;
    }

    QString range = sensorDescription;
    range.replace(QRegExp("^([^\\(\\)]*)\\(([^\\(\\)]*)\\).*$"), "\\2");

    if (sensorDataType == SDT_ENUM) {
        QStringList enums = range.split(',', QString::SkipEmptyParts);
        if (enums.length()==2) {
            enumFalse = enums.at(0);
            enumTrue = enums.at(1);
        } else {
            return;
        }
    } else {
        QStringList enums = range.split("..", QString::SkipEmptyParts);
        if (enums.length()==2) {
            fromValue = enums.at(0).toFloat();
            toValue = enums.at(1).toFloat();
        } else {
            return;
        }
    }

    QString stime = sensorDescription;
    stime.replace(QRegExp("^([^\\[\\]]*)\\[([^\\[\\]]*)\\].*"), "\\2");
    resetTime = stime.toInt();

    QString emstr = sensorDescription;
    emstr.replace(QRegExp("^([^\\[\\]]*)\\[([^\\[\\]]*)\\]([^\\|]*)\\|.*"), "\\3");
    sensorEM = emstr;

    QString namestr = sensorDescription;
    namestr.replace(QRegExp("^([^\\|]*)\\|(.*)"), "\\2");

    if (namestr.isEmpty()) return;

    sensorName = tr(namestr.toUtf8());

    shotTimer->setInterval(resetTime*1000);

    isValid = true;
}

void Sensor::reset_value()
{
    if (sensorDataType == SDT_ENUM) {
        if (sensorValue != enumFalse) {
            setValue(enumFalse);
            emit local_change();
        }
    } else {
        setValue(QString::number(fromValue, 'f', 6));
        emit local_change();
    }
}
