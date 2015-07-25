#include "sensor.h"

Sensor::Sensor(QObject *parent, QString sensor_description) : QObject(parent)
{
    block_id = 0;
    sensorValue = "";
    sensorDescription = sensor_description;
    upd_time = QDateTime::currentDateTime();
    log_file = NULL;
    log_buffer = new QList<SensorLogItem>();
    log_buffer_time_sub = 0;
    last_log_item.log_time = 0;
    shotTimer = new QTimer(this);
    shotTimer->setSingleShot(true);
    connect(shotTimer, SIGNAL(timeout()), this, SLOT(reset_value()));
    parseDescription();
}

Sensor::~Sensor()
{
    delete shotTimer;
    if (log_file) {
        writeLog(false);
        log_file->flush();
        delete log_file;
    }
    delete log_buffer;
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
    writeLog(true);
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

quint16 Sensor::getBlockID() const
{
    return block_id;
}

void Sensor::setBlockID(const quint16 &value)
{
    block_id = value;
}

QList<SensorLogItem> *Sensor::startLogDataTracking(quint64 time_sub)
{
    if (initLogFile() && log_file->isReadable()) {
        quint64 a, b, c, search;
        quint16 i_size = sizeof(SensorLogItem);
        SensorLogItem item;

        a = 0;
        b = log_file->size()/i_size - 1;
        c = 0;
        search = QDateTime::currentMSecsSinceEpoch() - time_sub;

        while (a<b-1) {
            c = (a+b) / 2;
            log_file->seek(c*i_size);
            if (log_file->read((char*)(void*)&item, i_size)<i_size) break;

            if (item.log_time > search) {
                b = c;
            } else if (item.log_time < search) {
                a = c;
            } else {
                break;
            }
        }

        log_file->seek(c*i_size);
        while (log_file->read((char*)(void*)&item, i_size)==i_size) {
            log_buffer->append(item);
        }
    }
    log_buffer_time_sub = time_sub;

    return log_buffer;
}

void Sensor::stopLogDataTracking()
{
    log_buffer->clear();
    log_buffer_time_sub = 0;
}

bool Sensor::writeLog(bool check_precision)
{
    bool result = false;

    SensorLogItem item;
    item.log_time = QDateTime::currentMSecsSinceEpoch();
    item.log_value = getFloatValue();

    if (!check_precision || !last_log_item.log_time || fabs(item.log_value-last_log_item.log_value)>SENSORS_LOG_BUFFER_PRECISION) {
        if (initLogFile() && log_file->isWritable()) {
            log_file->seek(log_file->size());
            unsigned bytes = log_file->write((char*)(void*)&item, sizeof(item));
            result = bytes==sizeof(item);
        }
        last_log_item = item;

        if (log_buffer_time_sub) {
            log_buffer->append(item);
            cutLogBuffer();
        }
    }

    return result;
}

void Sensor::cutLogBuffer()
{
    if (log_buffer->size()>2) {
        quint64 current_start = last_log_item.log_time - log_buffer_time_sub;
        unsigned int i;

        for(i=0; i<log_buffer->size() && log_buffer->at(i).log_time<current_start; i++);

        if (i<log_buffer->size() && i>0) {
            while (i>0) {
                log_buffer->removeFirst();
                i--;
            }
        }
    }
}

bool Sensor::initLogFile()
{
    if (!log_file) {
        QDir dir(QCoreApplication::instance()->applicationDirPath()+"/sensors_logs");
        if (!dir.exists()) {
            dir.mkpath(".");
        }
        log_file = new QFile(QCoreApplication::instance()->applicationDirPath()+"/sensors_logs/DS"+QString::number(block_id)+"_"+sensorLetter+".slog");
        log_file->open(QIODevice::ReadWrite);
    }
    return log_file && log_file->isOpen();
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
            sensorValue = enumFalse;
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
