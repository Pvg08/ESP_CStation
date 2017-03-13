#include "mainpccontroller.h"

MainPCController::MainPCController(QObject *parent, QString config_filename) : QObject(parent)
{
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
        if (units->at(i).thread) {
            if (units->at(i).thread->isRunning()) units->at(i).thread->terminate();
            if (units->at(i).generator) delete units->at(i).generator;
            delete units->at(i).thread;
        }
    }
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
        units->data()[unit_code].thread->listen(usbport, 30000);
    }
}

void MainPCController::showLogMessage(QString msg)
{
    emit logMessage(msg);
}
