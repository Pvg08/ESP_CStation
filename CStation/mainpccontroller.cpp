#include "mainpccontroller.h"

MainPCController::MainPCController(QObject *parent, QString config_filename) : QObject(parent)
{
    mode_state_tracking = 0;
    mode_state_indication = 0;
    mode_state_silence = 0;
    mode_state_control = 0;
    mode_state_security = 0;

    server = NULL;
    units = new QVector<PeripheralUnit>();
    for(int i=0; i<PERIPHERAL_UNITS_COUNT; i++ ) {
        PeripheralUnit adu;
        adu.generator = NULL;
        adu.thread = NULL;
        adu.usb_id = "";
        units->append(adu);
    }

    // create threads and generators
    units->data()[UNIT_MAIN].generator = new DataGeneratorMainController();
    units->data()[UNIT_LED_SCREEN].generator = new DataGeneratorLEDScreen();
    units->data()[UNIT_LSTRIP_SERVO].generator = new DataGeneratorServoLaser();
    units->data()[UNIT_LED_RING].generator = new DataGeneratorLEDRingRGB();

    for(int i=0; i<PERIPHERAL_UNITS_COUNT; i++ ) {
        if (units->at(i).generator) {
            units->data()[i].generator->setBaseTimeout(10000);
            units->data()[i].thread = new SerialFXWriter();
            units->data()[i].thread->setGenerator(units->data()[i].generator);
            connect(units->data()[i].thread, SIGNAL(response(QString)), this, SLOT(showLogMessage(QString)));
            connect(units->data()[i].thread, SIGNAL(error(QString)), this, SLOT(showLogMessage(QString)));
            connect(units->data()[i].thread, SIGNAL(timeout(QString)), this, SLOT(showLogMessage(QString)));
            connect(units->data()[i].thread, SIGNAL(log(QString)), this, SLOT(showLogMessage(QString)));
            //connect(units->data()[i].thread, SIGNAL(frame_play_confirmed()), this, SLOT(frame_played()));
            //connect(units->data()[i].thread, SIGNAL(frame_error()), this, SLOT(frame_error()));
        }
    }
    connect(units->data()[UNIT_MAIN].thread, SIGNAL(some_command(uint8_t,uint8_t,uint8_t,uint8_t)), this, SLOT(recieveMainCMD(uint8_t,uint8_t,uint8_t,uint8_t)));

    config_file = config_filename;
    QSettings settings(config_file, QSettings::IniFormat);

    server_port = settings.value("main/server_port", 51015).toInt();
    server_remote_port = settings.value("main/remote_port", 51016).toInt();
    server_evt_from = settings.value("main/event_time_from", 22).toInt();
    server_evt_to = settings.value("main/event_time_to", 8).toInt();

    setUSBPortForUnit(UNIT_MAIN, settings.value("units/main_unit.usb", "").toString());
    setUSBPortForUnit(UNIT_LED_SCREEN, settings.value("units/ledscreen_unit.usb", "").toString());
    setUSBPortForUnit(UNIT_LSTRIP_SERVO, settings.value("units/servo_unit.usb", "").toString());
    setUSBPortForUnit(UNIT_LED_RING, settings.value("units/ledring_unit.usb", "").toString());
}

MainPCController::~MainPCController()
{
    getReadyToClose(false);
    delete units;
    if (server) delete server;
}

Server *MainPCController::getServer()
{
    if (!server) restartServer();
    return server;
}

bool MainPCController::serverIsRunning()
{
    return !!server;
}

void MainPCController::restartServer()
{
    if (!server) {
        server = new Server();
        server->setPort(server_port);
        server->setRemotePort(server_remote_port);
        server->StartServer();
        server->setTimeEvtFrom(server_evt_from);
        server->setTimeEvtTo(server_evt_to);
        restartUnitThreads();
    } else {
        server->setPort(server_port);
        server->setRemotePort(server_remote_port);
        server->Reset();
        restartUnitThreads();
    }
}

void MainPCController::restartUnitThreads()
{
    for(int i=0; i<PERIPHERAL_UNITS_COUNT; i++ ) {
        if (!units->data()[i].usb_id.isEmpty() && units->data()[i].thread) {
            units->data()[i].thread->listen(units->data()[i].usb_id, 30000);
        }
    }
    last_rtc_send = QDateTime::currentDateTime();
    sendMainCMD(CMD_CMD_TURNINGON, 0, 0, 0, 0);
    sendMainCMD(CMD_CMD_SETRTCTIME, last_rtc_send.toMSecsSinceEpoch()/1000, 0, 0, 0);
}

void MainPCController::setServerPort(int value)
{
    server_port = value;
}

void MainPCController::setServerRemotePort(int value)
{
    server_remote_port = value;
}

void MainPCController::setServerEvtFrom(int value)
{
    server_evt_from = value;
    if (server) server->setTimeEvtFrom(value);
}

void MainPCController::setServerEvtTo(int value)
{
    server_evt_to = value;
    if (server) server->setTimeEvtTo(value);
}

int MainPCController::getServerPort() const
{
    return server_port;
}

int MainPCController::getServerRemotePort() const
{
    return server_remote_port;
}

int MainPCController::getServerEvtFrom() const
{
    return server_evt_from;
}

int MainPCController::getServerEvtTo() const
{
    return server_evt_to;
}

QString MainPCController::getUSBPortForUnit(int unit_code)
{
    return units->at(unit_code).usb_id;
}

void MainPCController::setUSBPortForUnit(int unit_code, QString usbport)
{
    units->data()[unit_code].usb_id = usbport;
    if (units->data()[unit_code].thread->isRunning()) {
        if (!usbport.isEmpty()) {
            units->data()[unit_code].thread->listen(usbport, 30000);
        } else {
            units->data()[unit_code].thread->do_stop();
        }
    }
}

void MainPCController::showLogMessage(QString msg)
{
    emit logMessage(msg);
}

void MainPCController::recieveMainCMD(uint8_t cmd, uint8_t param1, uint8_t param2, uint8_t param3)
{
    switch (cmd) {
    case CMD_CMD_TURNOFF:
        doTurnOff();
    break;
    case CMD_CMD_SETMODESTATE:
        setModeState(param1, param2);
    break;
    }
}

void MainPCController::setModeState(uint8_t mode_code, uint8_t mode_state)
{
    switch (mode_code) {
    case CMD_MODE_TRACKING:
        mode_state_tracking = mode_state;
    break;
    case CMD_MODE_INDICATION:
        mode_state_indication = mode_state;
    break;
    case CMD_MODE_SILENCE:
        mode_state_silence = mode_state;
    break;
    case CMD_MODE_CONTROL:
        mode_state_control = mode_state;
    break;
    case CMD_MODE_SECURITY:
        mode_state_security = mode_state;
    break;
    }
}

void MainPCController::sendMainCMD(uint8_t cmd, uint32_t param0, uint8_t param1, uint8_t param2, uint8_t param3)
{
    if (units->data()[UNIT_MAIN].thread && units->data()[UNIT_MAIN].thread->isRunning()) {
        ((DataGeneratorMainController*) units->data()[UNIT_MAIN].generator)->appendNextCommand(cmd, param0, param1, param2, param3);
    }
}

void MainPCController::doTurnOff()
{
    emit turningOff();
    getReadyToClose(true);
    if (server) {
        server->StopServer();
        delete server;
        server = NULL;
    }
    QEventLoop loop;
    while (true) {
        sendMainCMD(CMD_CMD_TURNOFFREADY, 0, 0, 0, 0);
        QTimer::singleShot(1000, &loop, SLOT(quit()));
        loop.exec();
    }
}

void MainPCController::getReadyToClose(bool dont_close_mainc_thread)
{
    if (!config_file.isEmpty()) {
        QSettings settings(config_file, QSettings::IniFormat);
        settings.setValue("main/server_port", server_port);
        settings.setValue("main/remote_port", server_remote_port);
        settings.setValue("main/event_time_from", server_evt_from);
        settings.setValue("main/event_time_to", server_evt_to);
        settings.setValue("units/main_unit.usb", getUSBPortForUnit(UNIT_MAIN));
        settings.setValue("units/ledscreen_unit.usb", getUSBPortForUnit(UNIT_LED_SCREEN));
        settings.setValue("units/servo_unit.usb", getUSBPortForUnit(UNIT_LSTRIP_SERVO));
        settings.setValue("units/ledring_unit.usb", getUSBPortForUnit(UNIT_LED_RING));
    }
    for(int i=0; i<PERIPHERAL_UNITS_COUNT; i++ ) {
        if (units->at(i).thread && (!dont_close_mainc_thread || !(dont_close_mainc_thread && i==UNIT_MAIN))) {
            if (units->at(i).thread->isRunning()) units->at(i).thread->terminate();
            if (units->at(i).generator) delete units->at(i).generator;
            delete units->at(i).thread;
            units->data()[i].thread = NULL;
        }
    }
}
