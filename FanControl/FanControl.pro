QT += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = fancontrol
TEMPLATE = app
win32:RC_ICONS += res/fan.ico
VERSION = 0.1
QMAKE_TARGET_PRODUCT = Fan Control
QMAKE_TARGET_COMPANY = Bryce
QMAKE_TARGET_DESCRIPTION = Fan Control
QMAKE_TARGET_COPYRIGHT =

SOURCES += main.cpp\
    temperaturereader.cpp \
    smc.cpp \
    qxtspanslider.cpp \
    fanspeedwidget.cpp \
    mainwindow.cpp \
    preferences.cpp \
    fancontroller.cpp \
    temperaturewindow.cpp \
    aboutdialog.cpp \
    preferencesdialog.cpp \
    temperatureupdatetimer.cpp

HEADERS += \
    temperaturereader.h \
    smc.h \
    qxtspanslider.h \
    qxtspanslider_p.h \
    fanspeedwidget.h \
    mainwindow.h \
    preferences.h \
    fancontroller.h \
    temperaturewindow.h \
    qxtglobal.h \
    nvapi.h \
    debug.h \
    aboutdialog.h \
    preferencesdialog.h \
    temperatureupdatetimer.h

FORMS += \
    mainwindow.ui \
    temperaturewindow.ui \
    preferencesdialog.ui \
    aboutdialog.ui

RESOURCES += \
    application.qrc

LIBS += \
    -L$$PWD/lib/ -lnvapi

PRE_TARGETDEPS += \
    $$PWD/lib/nvapi.lib
