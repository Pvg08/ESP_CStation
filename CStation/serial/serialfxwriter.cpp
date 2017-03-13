#include "serialfxwriter.h"

QT_USE_NAMESPACE

SerialFXWriter::SerialFXWriter(QObject *parent)
    : QThread(parent), waitTimeout(0), quit(false)
{
    state_index = 0;
    request_write_position = 0;
    request_confirm_position = 0;
    request_generate_position = 0;
    confirmed_last_play_index = 0;
    confirmed_last_write_index = 0;
}

SerialFXWriter::~SerialFXWriter()
{
    mutex.lock();
    quit = true;
    mutex.unlock();
    wait();
}

void SerialFXWriter::listen(const QString &portName, int waitTimeout, DataGenerator* c_generator)
{
    QMutexLocker locker(&mutex);
    this->portName = portName;
    this->waitTimeout = waitTimeout;
    if (c_generator) generator = c_generator;
    if (!generator) return;
    confirmed_last_play_index = confirmed_last_write_index = 0;
    half_buf_size = generator->getBufferSize() / 2;
    reset_states = true;
    quit = false;
    if (!isRunning()) start(QThread::HighestPriority);
}

void SerialFXWriter::run()
{
    bool currentPortNameChanged = false;
    bool data_written = false;

    mutex.lock();
    QString currentPortName = "";
    if (currentPortName != portName) {
        currentPortName = portName;
        currentPortNameChanged = true;
    }
    int currentWaitTimeout = waitTimeout;
    mutex.unlock();

    QSerialPort serial;

    emit log("Thread started...");

    while (!quit) {

        if (currentPortNameChanged) {
            serial.close();
            #if defined(WIN32) || defined(__WATCOMC__) || defined(_WIN32) || defined(__WIN32__)
                serial.setPortName(currentPortName);
            #else
                serial.setPortName("/dev/"+currentPortName);
            #endif;

            if (!serial.open(QIODevice::ReadWrite)) {
                emit error(tr("Can't open %1, error code %2")
                           .arg(portName).arg(serial.error()));
                return;
            }
            serial.setBaudRate(QSerialPort::Baud115200);
            emit log("Baud rate: "+QString::number(serial.baudRate()));
            emit log("Writeable: "+QString::number(serial.isWritable()));
            emit log("Readable: "+QString::number(serial.isReadable()));
            emit log("dataBits: "+QString::number(serial.dataBits()));
            emit log("stopBits: "+QString::number(serial.stopBits()));
            emit log("parity: "+QString::number(serial.parity()));
        }

        if (reset_states) {
            resetBuffers();
            serial.write(*send_buffer.at(0));
            if (!serial.waitForBytesWritten(waitTimeout)) {
                emit timeout(tr("Wait write request timeout %1")
                             .arg(QTime::currentTime().toString()));
            }
            reset_states = false;
        }

        fillBuffer();

        if (request_write_position != request_generate_position) {
            while(request_write_position != request_generate_position) {
                request_write_position = getNextPosition(request_write_position);
                serial.write(*send_buffer.at(request_write_position));
                data_written = true;
            }
            request_write_position = request_generate_position;
        }

        if (data_written) {
            if (!serial.waitForBytesWritten(waitTimeout)) {
                emit timeout(tr("Wait write request timeout %1")
                             .arg(QTime::currentTime().toString()));
            }
            data_written = false;
        }

        // read response
        if (serial.waitForReadyRead(currentWaitTimeout)) {
            emit log("Ready read...");
            QByteArray responseData = serial.readAll();
            while (serial.waitForReadyRead(10))
                responseData += serial.readAll();

            responseCheck(responseData);
        } else {
            emit timeout(tr("Wait read response timeout %1")
                         .arg(QTime::currentTime().toString()));
        }

        mutex.lock();
        if (currentPortName != portName) {
            currentPortName = portName;
            currentPortNameChanged = true;
        } else {
            currentPortNameChanged = false;
        }
        currentWaitTimeout = waitTimeout;
        mutex.unlock();
    }
    emit log("Thread closed...");
}

void SerialFXWriter::do_stop()
{
    if (isRunning()) quit = true;
}

DataGenerator *SerialFXWriter::getGenerator() const
{
    return generator;
}

void SerialFXWriter::setGenerator(DataGenerator *value)
{
    generator = value;
}

void SerialFXWriter::resetBuffers()
{
    emit log(QString("Resetting buffers (size = %1)...").arg(generator->getDataSize()));
    state_index = 0;
    if (send_buffer.size()) {
        for(int i=0; i<send_buffer.size(); i++) {
            delete send_buffer[i];
        }
        send_buffer.clear();
    }
    while (send_buffer.size() < generator->getBufferSize()) {
        QByteArray* bytearray = new QByteArray();
        bytearray->resize(generator->getDataSize());
        generator->fillEmptyState(state_index, bytearray);
        send_buffer.append(bytearray);
    }
    ((StateStruct*)send_buffer[0]->data())->hash = 0;
    ((StateStruct*)send_buffer[0]->data())->command = CMD_RESET;
    ((StateStruct*)send_buffer[0]->data())->timeout = 0;
    request_write_position = request_confirm_position = request_generate_position = send_buffer.size()-1;
}

void SerialFXWriter::fillBuffer()
{
    unsigned interval = 0;

    if (request_confirm_position < request_generate_position) {
        interval = request_generate_position - request_confirm_position;
    } else if (request_confirm_position == request_generate_position) {
        if (request_write_position != request_generate_position) {
            interval = send_buffer.size();
        } else {
            interval = 0;
        }
    } else {
        interval = request_confirm_position + send_buffer.size() - request_generate_position;
    }

    emit log("Buffer interval: "+QString::number(interval));

    if (interval<half_buf_size) {
        for(interval=0; interval<half_buf_size; interval++) {
            request_generate_position = getNextPosition(request_generate_position);
            generator->fillNextState(state_index++, send_buffer[request_generate_position]);
            emit log("Generating block "+QString::number(state_index-1)+"...");
        }
    }
    emit log("Buffer genpos: "+QString::number(request_generate_position));
}

void SerialFXWriter::responseCheck(QByteArray response)
{
    emit this->response(((QString) response.toHex()) + " (" + QString::fromLocal8Bit(response) + ")");

    bool confirm_set = false;
    uint8_t data[4];
    for(int i=0; i<response.size()-6; i++) {
        if (response.at(i) == 'C' && response.at(i+1) == 'S' && response.at(i+2) == 'P') {
            data[0] = response.at(i+3);
            data[1] = response.at(i+4);
            data[2] = response.at(i+5);
            data[3] = response.at(i+6);
            if (confirmed_last_play_index != *((uint32_t*)(void*)data)) {
                emit frame_play_confirmed();
                confirmed_last_play_index = *((uint32_t*)(void*)data);
            }
            confirm_set = true;
            i+=6;
            emit log("SET confirmed_last_play_index: "+QString::number(confirmed_last_play_index));
        }
        if (response.at(i) == 'C' && response.at(i+1) == 'S' && response.at(i+2) == 'W') {
            data[0] = response.at(i+3);
            data[1] = response.at(i+4);
            data[2] = response.at(i+5);
            data[3] = response.at(i+6);
            confirmed_last_write_index = *((uint32_t*)(void*)data);
            confirm_set = true;
            i+=6;
            emit log("SET confirmed_last_write_index: "+QString::number(confirmed_last_write_index));
        }
    }
    if (confirm_set && (confirmed_last_write_index-confirmed_last_play_index)<half_buf_size) {
        setConfirmPosition(confirmed_last_write_index);
    }
}

void SerialFXWriter::setConfirmPosition(uint32_t confirm_write_position)
{
    unsigned i;
    for(i=0; i<send_buffer.size() && ((StateStruct*)(void*)send_buffer.at(i)->data())->state_index != confirm_write_position; i++) {
        emit log("CPOS["+QString::number(i)+"]: "+QString::number(((StateStruct*)(void*)send_buffer.at(i)->data())->state_index) + " <> " + QString::number(confirm_write_position));
    }
    if (i<send_buffer.size()) {
        if (request_write_position != i) emit frame_error();
        request_confirm_position = i;
        request_write_position = i;
        emit log("Confirmed request_confirm_position="+QString::number(request_confirm_position) +", request_write_position="+QString::number(request_write_position));
    } else {
        emit log("Confirmed request_confirm_position not found");
        emit frame_error();
    }
}

unsigned SerialFXWriter::getNextPosition(unsigned cur_position)
{
    cur_position++;
    if (cur_position>=send_buffer.size()) cur_position = 0;
    return cur_position;
}
