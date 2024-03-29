#-------------------------------------------------
#
# Project created by QtCreator 2019-10-29T09:09:39
#
#-------------------------------------------------

QT       += core gui scxml xml

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = SalusRT500
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# Uncomment for external Scxml Editor Debugging
# https://github.com/alexzhornyak/ScxmlEditor-Tutorial#external-debugging
#DEFINES += _SCXML_EXTERN_MONITOR_

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
        main.cpp \
        mainwindow.cpp \
        salusdatamodel.cpp

HEADERS += \
        mainwindow.h \
        salusdatamodel.h

FORMS += \
        mainwindow.ui

DISTFILES +=

RESOURCES += \
    salusrt500.qrc

STATECHARTS += \
    SalusRT500Logic.scxml
