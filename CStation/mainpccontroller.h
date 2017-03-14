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

/* Data Exchange Params */
#define CMD_CMD_TURNINGON 0x01
#define CMD_CMD_TURNOFFBEGIN 0x03
#define CMD_CMD_TURNOFFREADY 0x04
#define CMD_CMD_TURNOFF 0x05
#define CMD_CMD_SETMODESTATE 0x10
#define CMD_CMD_SETRTCTIME 0x20

#define CMD_MODE_TRACKING 0x11
#define CMD_MODE_INDICATION 0x12
#define CMD_MODE_SILENCE 0x13
#define CMD_MODE_CONTROL 0x14
#define CMD_MODE_SECURITY 0x15
/* /Data Exchange Params */

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
    void turningOff();

private slots:
    void showLogMessage(QString msg);
    void recieveMainCMD(uint8_t cmd, uint8_t param1, uint8_t param2, uint8_t param3);

private:
    Server* server;
    QString config_file;
    QVector<PeripheralUnit>* units;
    QDateTime last_rtc_send;

    uint8_t mode_state_tracking;
    uint8_t mode_state_indication;
    uint8_t mode_state_silence;
    uint8_t mode_state_control;
    uint8_t mode_state_security;

    int server_port = 0;
    int server_remote_port = 0;
    int server_evt_from = 0;
    int server_evt_to = 0;

    void sendMainCMD(uint8_t cmd, uint32_t param0, uint8_t param1, uint8_t param2, uint8_t param3);
    void doTurnOff();
    void getReadyToClose(bool dont_close_mainc_thread);
    void setModeState(uint8_t mode_code, uint8_t mode_state);
};

#endif // MAINPCCONTROLLER_H
