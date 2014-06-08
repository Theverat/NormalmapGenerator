#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMessageBox>
#include <QFileDialog>
#include <QElapsedTimer>
#include <QDesktopServices>
#include <QUrl>
#include <iostream>
#include "normalmapgenerator.h"
#include "specularmapgenerator.h"
#include "queueitem.h"

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
    ~MainWindow();
    
private:
    Ui::MainWindow *ui;
    QImage input;
    QImage channelIntensity;
    QImage normalmap;
    QImage specmap;
    QImage displacementmap;
    QString loadedImagePath;
    QString exportPath;
    int lastCalctime_normal;
    int lastCalctime_specular;
    int lastCalctime_displace;
    bool stopQueue;

    void calcNormal();
    void calcSpec();
    void calcDisplace();
    QString generateElapsedTimeMsg(int calcTimeMs, QString mapType);
    void connectSignalSlots();
    void displayCalcTime(int calcTime_ms, QString mapType, int duration_ms);
    void enableAutoupdate(bool on);
    void addImageToQueue(QUrl url);
    void addImageToQueue(QList<QUrl> urls);
    void saveQueueProcessed(QUrl folderPath);
    void save(QString filename);
    bool load(QString filename);

private slots:
    void loadUserFilePath();
    void loadSingleDropped(QUrl url);
    void loadMultipleDropped(QList<QUrl> urls);
    void calcNormalAndPreview();
    void calcSpecAndPreview();
    void calcDisplaceAndPreview();
    void processQueue();
    void stopProcessingQueue();
    void saveUserFilePath();
    void preview();
    void preview(int tab);
    void zoomIn();
    void zoomOut();
    void resetZoom();
    void fitInView();
    void autoUpdate();
    void displayChannelIntensity();
    void openExportFolder();
    void removeImagesFromQueue();
    void changeOutputPathQueue();
};

#endif // MAINWINDOW_H
