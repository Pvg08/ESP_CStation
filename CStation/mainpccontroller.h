#ifndef MAINPCCONTROLLER_H
#define MAINPCCONTROLLER_H

#include <QObject>
#include <QVector>

#include "server.h"

#include "./serial/serialfxwriter.h"
#include "./serial/datageneratorledscreen.h"
#include "./serial/datageneratorledring.h"
#include "./serial/datageneratorledringrgb.h"
#include "./serial/datageneratorledrgbw.h"
#include "./serial/datageneratorservolaser.h"
#include "./serial/datageneratormaincontroller.h"

#define PERIPHERAL_UNITS_COUNT 4
#define UNIT_MAIN 0
#define UNIT_LED_SCREEN 1
#define UNIT_LSTRIP_SERVO 2
#define UNIT_LED_RING 3

typedef struct
{
    QString usb_id;
    SerialFXWriter* thread;
    DataGenerator* generator;
} PeripheralUnit;

class MainPCController : public QObject
{
    Q_OBJECT
public:
    explicit MainPCController(QObject *parent, QString config_filename);
    ~MainPCController();

    Server *getServer();
    bool serverIsRunning();
    void restartServer();
    void restartUnitThreads();

    void setServerPort(int value);
    void setServerRemotePort(int value);
    void setServerEvtFrom(int value);
    void setServerEvtTo(int value);
    int getServerPort() const;
    int getServerRemotePort() const;
    int getServerEvtFrom() const;
    int getServerEvtTo() const;

    QString getUSBPortForUnit(int unit_code);
    void setUSBPortForUnit(int unit_code, QString usbport);
signals:
    void logMessage(QString);

private slots:
    void showLogMessage(QString msg);

private:
    Server* server;
    QString config_file;
    QVector<PeripheralUnit>* units;

    int server_port = 0;
    int server_remote_port = 0;
    int server_evt_from = 0;
    int server_evt_to = 0;
};

#endif // MAINPCCONTROLLER_H
