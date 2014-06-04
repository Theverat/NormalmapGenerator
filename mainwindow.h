#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMessageBox>
#include <QFileDialog>
#include <QElapsedTimer>
#include <iostream>
#include "normalmapgenerator.h"
#include "specularmapgenerator.h"

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
    int lastCalctime_normal;
    int lastCalctime_specular;
    int lastCalctime_displace;
    QString generateElapsedTimeMsg(int calcTimeMs, QString mapType);

private slots:
    void load();
    void calcNormal();
    void calcSpec();
    void calcDisplace();
    void save();
    void saveAll();
    void preview();
    void preview(int tab);
    void zoomIn();
    void zoomOut();
    void resetZoom();
    void fitInView();
    void autoUpdate();
    void displayChannelIntensity();
};

#endif // MAINWINDOW_H
