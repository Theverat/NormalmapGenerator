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
    sobel.cpp \
    intensitymap.cpp

HEADERS  += mainwindow.h \
    sobel.h \
    intensitymap.h

FORMS    += mainwindow.ui
