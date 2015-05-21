################################################################################
#   Copyright (C) 2015 by Simon Wendsche                                       #
#                                                                              #
#   This file is part of NormalmapGenerator.                                   #
#                                                                              #
#   NormalmapGenerator is free software; you can redistribute it and/or modify #
#   it under the terms of the GNU General Public License as published by       #
#   the Free Software Foundation; either version 3 of the License, or          #
#   (at your option) any later version.                                        #
#                                                                              #
#   NormalmapGenerator is distributed in the hope that it will be useful,      #
#   but WITHOUT ANY WARRANTY; without even the implied warranty of             #
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the              #
#   GNU General Public License for more details.                               #
#                                                                              #
#   You should have received a copy of the GNU General Public License          #
#   along with this program.  If not, see <http://www.gnu.org/licenses/>.      #
#                                                                              #
#   Sourcecode: https://github.com/Theverat/NormalmapGenerator                 #
################################################################################

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = NormalmapGenerator
TEMPLATE = app

QMAKE_CXXFLAGS += -fopenmp -std=c++11
LIBS += -fopenmp

SOURCES += main.cpp\
        src_gui/mainwindow.cpp \
    src_generators/intensitymap.cpp \
    src_generators/normalmapgenerator.cpp \
    src_generators/specularmapgenerator.cpp \
    src_gui/graphicsscene.cpp \
    src_gui/graphicsview.cpp \
    src_gui/queueitem.cpp \
    src_gui/queuemanager.cpp \
    src_generators/gaussianblur.cpp \
    src_generators/boxblur.cpp \
    src_generators/ssaogenerator.cpp \
    src_gui/aboutdialog.cpp \
    src_gui/listwidget.cpp

HEADERS  += src_gui/mainwindow.h \
    src_generators/intensitymap.h \
    src_generators/normalmapgenerator.h \
    src_generators/specularmapgenerator.h \
    src_gui/graphicsscene.h \
    src_gui/graphicsview.h \
    src_gui/queueitem.h \
    src_gui/queuemanager.h \
    src_generators/gaussianblur.h \
    src_generators/boxblur.h \
    src_generators/ssaogenerator.h \
    src_gui/aboutdialog.h \
    src_gui/listwidget.h

FORMS    += src_gui/mainwindow.ui \
    src_gui/aboutdialog.ui
