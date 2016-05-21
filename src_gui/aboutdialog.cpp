/********************************************************************************
 *   Copyright (C) 2015 by Simon Wendsche                                       *
 *                                                                              *
 *   This file is part of NormalmapGenerator.                                   *
 *                                                                              *
 *   NormalmapGenerator is free software; you can redistribute it and/or modify *
 *   it under the terms of the GNU General Public License as published by       *
 *   the Free Software Foundation; either version 3 of the License, or          *
 *   (at your option) any later version.                                        *
 *                                                                              *
 *   NormalmapGenerator is distributed in the hope that it will be useful,      *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of             *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the              *
 *   GNU General Public License for more details.                               *
 *                                                                              *
 *   You should have received a copy of the GNU General Public License          *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.      *
 *                                                                              *
 *   Sourcecode: https://github.com/Theverat/NormalmapGenerator                 *
 ********************************************************************************/

#include "aboutdialog.h"
#include "ui_aboutdialog.h"

#include <QFile>
#include <QDir>
#include <QDesktopServices>
#include <QColorDialog>

AboutDialog::AboutDialog(QWidget *parent, MainWindow *mainwindow) :
    QDialog(parent),
    mainwindow(mainwindow),
    ui(new Ui::AboutDialog)
{
    ui->setupUi(this);

    connect(ui->pushButton_sourcecode, SIGNAL(clicked()), this, SLOT(openSourcecodeLink()));
    connect(ui->pushButton_latestVersion, SIGNAL(clicked()), this, SLOT(openLatestVersionLink()));

    connect(ui->radioButton_useCustomColors, SIGNAL(toggled(bool)), mainwindow, SLOT(setUseCustomUiColors(bool)));
    connect(ui->pushButton_mainColor, SIGNAL(clicked()), this, SLOT(showMainColorDialog()));
    connect(ui->pushButton_textColor, SIGNAL(clicked()), this, SLOT(showTextColorDialog()));
    connect(ui->pushButton_graphicsViewColor, SIGNAL(clicked()), this, SLOT(showGraphicsViewColorDialog()));
    connect(ui->pushButton_resetColorsToDefault, SIGNAL(clicked()), mainwindow, SLOT(resetUiColors()));

    ui->radioButton_useCustomColors->setChecked(mainwindow->getUseCustomUiColors());
}

AboutDialog::~AboutDialog()
{
    delete ui;
}

void AboutDialog::openSourcecodeLink() {
    QDesktopServices::openUrl(QUrl("https://github.com/Theverat/NormalmapGenerator"));
}

void AboutDialog::openLatestVersionLink() {
    QDesktopServices::openUrl(QUrl("https://github.com/Theverat/NormalmapGenerator/releases"));
}

void AboutDialog::showMainColorDialog() {
    QColorDialog *colorDialog = new QColorDialog(mainwindow->getUiColorMain());
    colorDialog->setWindowFlags(Qt::SubWindow);
    connect(colorDialog, SIGNAL(currentColorChanged(QColor)), mainwindow, SLOT(setUiColorMain(QColor)));
    colorDialog->show();
}

void AboutDialog::showTextColorDialog() {
    QColorDialog *colorDialog = new QColorDialog(mainwindow->getUiColorText());
    colorDialog->setWindowFlags(Qt::SubWindow);
    connect(colorDialog, SIGNAL(currentColorChanged(QColor)), mainwindow, SLOT(setUiColorText(QColor)));
    colorDialog->show();
}

void AboutDialog::showGraphicsViewColorDialog() {
    QColorDialog *colorDialog = new QColorDialog(mainwindow->getUiColorGraphicsView());
    colorDialog->setWindowFlags(Qt::SubWindow);
    connect(colorDialog, SIGNAL(currentColorChanged(QColor)), mainwindow, SLOT(setUiColorGraphicsView(QColor)));
    colorDialog->show();
}
