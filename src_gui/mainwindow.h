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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QUrl>
#include "queueitem.h"
#include "src_generators/intensitymap.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    enum Channel {
        RED,
        GREEN,
        BLUE,
        ALPHA
    };

    explicit MainWindow(QWidget *parent = 0);
    void closeEvent(QCloseEvent* event);
    ~MainWindow();
    bool getUseCustomUiColors();
    QColor getUiColorMain();
    QColor getUiColorText();
    QColor getUiColorGraphicsView();

public slots:
    void setUseCustomUiColors(bool value);
    void setUiColorMain(QColor value);
    void setUiColorText(QColor value);
    void setUiColorGraphicsView(QColor value);
    void resetUiColors();
    
private:
    Ui::MainWindow *ui;
    QImage input;
    QImage channelIntensity;
    QImage normalmap;
    QImage specmap;
    QImage displacementmap;
    QImage ssaomap;
    QImage normalmapRawIntensity;
    QUrl loadedImagePath;
    QUrl exportPath;
    int lastCalctime_normal;
    int lastCalctime_specular;
    int lastCalctime_displace;
    int lastCalctime_ssao;
    bool stopQueue;
    QStringList supportedImageformats;
    bool useCustomUiColors;
    QColor uiColorMainDefault;
    QColor uiColorTextDefault;
    QColor uiColorGraphicsViewDefault;
    QColor uiColorMain;
    QColor uiColorText;
    QColor uiColorGraphicsView;

    bool setExportPath(QUrl path);
    void calcNormal();
    void calcSpec();
    void calcDisplace();
    void calcSsao();
    QString generateElapsedTimeMsg(int calcTimeMs, QString mapType);
    void connectSignalSlots();
    void hideAdvancedSettings();
    void displayCalcTime(int calcTime_ms, QString mapType, int duration_ms);
    void enableAutoupdate(bool on);
    void addImageToQueue(QUrl url);
    void addImageToQueue(QList<QUrl> urls);
    void saveQueueProcessed(QUrl folderPath);
    void save(QUrl url);
    bool load(QUrl url);
    void loadAllFromDir(QUrl url);
    int calcPercentage(int value, int percentage);
    void setUiColors();
    void writeSettings();
    void readSettings();

private slots:
    void loadUserFilePath();
    void loadSingleDropped(QUrl url);
    void loadMultipleDropped(QList<QUrl> urls);
    void calcNormalAndPreview();
    void calcSpecAndPreview();
    void calcDisplaceAndPreview();
    void calcSsaoAndPreview();
    void processQueue();
    void stopProcessingQueue();
    void saveUserFilePath();
    void preview();
    void preview(int tab);
    void switchToTab1();
    void switchToTab2();
    void switchToTab3();
    void switchToTab4();
    void zoomIn();
    void zoomOut();
    void resetZoom();
    void fitInView();
    void autoUpdate();
    void displayChannelIntensity();
    void openExportFolder();
    void removeImagesFromQueue();
    void changeOutputPathQueueDialog();
    void editOutputPathQueue();
    void queueItemDoubleClicked(QListWidgetItem *item);
    void normalmapSizeChanged();
    void showAboutDialog();
    void on_horizontalSlider_valueChanged(int value);
    void on_horizontalSlider_2_valueChanged(int value);
    void on_checkBox_clicked(bool checked);
    void on_horizontalSlider_3_valueChanged(int value);
};

#endif // MAINWINDOW_H
