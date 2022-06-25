#-------------------------------------------------
#
# Project created by QtCreator 2014-06-02T09:49:36
#
#-------------------------------------------------

QT       += core gui network

include($$PWD/QtZeroConf/qtzeroconf.pri)

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

INCLUDEPATH += ../light_controller/common_includes ../kennel_fan_controller ../avahi ../avahi/avahi-qt

TARGET = light_controller_remote_ui
TEMPLATE = app

SOURCES += main.cpp\
        hex_dump.c \
        light_controller_remote_ui.cpp \
        ssl_client.cpp

HEADERS  += \
        hex_dump.h \
        light_controller_remote_ui.h \
        ssl_client.h

RESOURCES += light_controller_remote_ui.qrc

FORMS    += light_controller_remote_ui.ui

LIBS += -lavahi-client
LIBS += -lavahi-common
#LIBS += -lavahi-qt5

CONFIG += mobility
MOBILITY =

# install
target.path = /afs/aaker.org/home/kdaaker/src/light_controller/ligh_controller_remote_ui
INSTALLS += target

