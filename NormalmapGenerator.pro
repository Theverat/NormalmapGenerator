#-------------------------------------------------
#
# Project created by QtCreator 2014-06-02T09:17:09
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = NormalmapGenerator
TEMPLATE = app

QMAKE_CXXFLAGS += -fopenmp -std=c++11
LIBS += -fopenmp

SOURCES += main.cpp\
        mainwindow.cpp \
    intensitymap.cpp \
    normalmapgenerator.cpp \
    specularmapgenerator.cpp \
    graphicsscene.cpp \
    graphicsview.cpp \
    queueitem.cpp \
    queuemanager.cpp \
    gaussianblur.cpp \
    boxblur.cpp \
    ssaogenerator.cpp \
    aboutdialog.cpp

HEADERS  += mainwindow.h \
    intensitymap.h \
    normalmapgenerator.h \
    specularmapgenerator.h \
    graphicsscene.h \
    graphicsview.h \
    queueitem.h \
    queuemanager.h \
    gaussianblur.h \
    boxblur.h \
    ssaogenerator.h \
    aboutdialog.h

FORMS    += mainwindow.ui \
    aboutdialog.ui
