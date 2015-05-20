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
    ~MainWindow();
    
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
    int calcPercentage(int value, int percentage);

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
    void zoomIn();
    void zoomOut();
    void resetZoom();
    void fitInView();
    void autoUpdate();
    void displayChannelIntensity();
    void openExportFolder();
    void removeImagesFromQueue();
    void changeOutputPathQueue();
    void updateQueueExportOptions();
    void queueItemDoubleClicked(QListWidgetItem *item);
    void normalmapSizeChanged();
    void showAboutDialog();
};

#endif // MAINWINDOW_H
