#-------------------------------------------------
#
# Project created by QtCreator 2018-09-27T19:47:27
#
#-------------------------------------------------

QT       += core gui
QT       += network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = IcmDevelopmentToolbox
TEMPLATE = app
RC_ICONS = ../resources/appicon.ico

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += main.cpp \
        mainwindow.cpp \
    bitmapprocess.cpp \
    bmp.cpp \
    applauncher.cpp

HEADERS  += mainwindow.h \
    defaultFlashDriverCode.h \
    crc16.h \
    defaultEraseEepromCode.h \
    defaultM1SeriesBootCode.h \
    defaultT1SeriesBootCode.h \
    defaultS51evflBootCode.h \
    defaultA13tevflBootCode.h \
    bitmapprocess.h \
    bmp.h \
    applauncher.h

FORMS    += mainwindow.ui \
    bitmapprocess.ui
