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

AboutDialog::AboutDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AboutDialog)
{
    ui->setupUi(this);

    connect(ui->pushButton_sourcecode, SIGNAL(clicked()), this, SLOT(openSourcecodeLink()));
    connect(ui->pushButton_latestVersion, SIGNAL(clicked()), this, SLOT(openLatestVersionLink()));
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
