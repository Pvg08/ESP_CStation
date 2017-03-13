#ifndef SERIALFXWRITER_H
#define SERIALFXWRITER_H

#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <QVector>
#include <QDateTime>
#include <QtSerialPort/QSerialPort>

#include "./datagenerator.h"

class SerialFXWriter : public QThread
{
    Q_OBJECT

public:
    SerialFXWriter(QObject *parent = 0);
    ~SerialFXWriter();

    void listen(const QString &portName, int waitTimeout, DataGenerator* c_generator = 0);
    void run();
    void do_stop();

    DataGenerator *getGenerator() const;
    void setGenerator(DataGenerator *value);

signals:
    void response(const QString &s);
    void error(const QString &s);
    void timeout(const QString &s);
    void log(const QString &s);
    void frame_play_confirmed();
    void frame_error();
    void some_command(uint8_t param1, uint8_t param2, uint8_t param3, uint8_t param4);

private:
    QString portName;
    QVector<QByteArray*> send_buffer;
    unsigned request_write_position, request_confirm_position, request_generate_position;
    int waitTimeout;
    QMutex mutex;
    bool quit;
    DataGenerator* generator;
    uint32_t state_index;
    uint32_t confirmed_last_play_index, confirmed_last_write_index;
    unsigned half_buf_size;
    bool reset_states;

    void resetBuffers();
    void fillBuffer();
    void responseCheck(QByteArray response);

    void setConfirmPosition(uint32_t confirm_write_position);
    unsigned getNextPosition(unsigned cur_position);
};

#endif // SERIALFXWRITER_H
