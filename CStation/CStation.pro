#-------------------------------------------------
#
# Project created by QtCreator 2015-07-10T17:22:35
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = CStation
TEMPLATE = app

SOURCES += main.cpp\
    mainwindow.cpp \
    server.cpp \
    clientblock.cpp \
    sensor.cpp \
    clientaction.cpp \
    widgets/sensorblock.cpp \
    widgets/sensorsdisplayform.cpp \
    widgets/msensordrawsurface.cpp

HEADERS  += mainwindow.h \
    server.h \
    clientblock.h \
    sensor.h \
    clientaction.h \
    widgets/sensorblock.h \
    widgets/sensorsdisplayform.h \
    widgets/msensordrawsurface.h

FORMS    += mainwindow.ui \
    widgets/sensorblock.ui \
    widgets/sensorsdisplayform.ui

TRANSLATIONS += translations/CStation_ru_RU.ts
