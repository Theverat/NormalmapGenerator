#-------------------------------------------------
#
# Project created by QtCreator 2014-06-02T09:17:09
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = NormalmapGenerator
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    intensitymap.cpp \
    normalmapgenerator.cpp \
    specularmapgenerator.cpp

HEADERS  += mainwindow.h \
    intensitymap.h \
    normalmapgenerator.h \
    specularmapgenerator.h

FORMS    += mainwindow.ui
