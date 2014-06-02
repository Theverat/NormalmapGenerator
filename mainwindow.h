#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMessageBox>
#include <QFileDialog>
#include "sobel.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    
private:
    Ui::MainWindow *ui;
    QImage input;
    QImage result;
    QString loadedImagePath;

private slots:
    void load();
    void calc();
    void save();
    void preview();
    void zoomIn();
    void zoomOut();
    void resetZoom();
    void fitInView();
};

#endif // MAINWINDOW_H
