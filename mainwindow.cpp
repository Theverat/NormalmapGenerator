#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(ui->pushButton_load, SIGNAL(clicked()), this, SLOT(load()));
    connect(ui->pushButton_calc, SIGNAL(clicked()), this, SLOT(calc()));
    connect(ui->pushButton_save, SIGNAL(clicked()), this, SLOT(save()));
    connect(ui->radioButton_generated, SIGNAL(clicked()), this, SLOT(preview()));
    connect(ui->radioButton_input, SIGNAL(clicked()), this, SLOT(preview()));
    connect(ui->pushButton_zoomIn, SIGNAL(clicked()), this, SLOT(zoomIn()));
    connect(ui->pushButton_zoomOut, SIGNAL(clicked()), this, SLOT(zoomOut()));
    connect(ui->pushButton_resetZoom, SIGNAL(clicked()), this, SLOT(resetZoom()));
    connect(ui->pushButton_fitInView, SIGNAL(clicked()), this, SLOT(fitInView()));

    QGraphicsScene *scene = new QGraphicsScene();
    ui->graphicsView->setScene(scene);
    scene->clear();
    scene->setBackgroundBrush(QBrush(Qt::darkGray));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::load() {
    QString filename = QFileDialog::getOpenFileName(this, "Open Image File", QDir::currentPath(), "Image Formats (*.png *.jpg *.jpeg *.tiff *.ppm *.bmp *.xpm)");
    if(filename.isEmpty())
        return;
    loadedImagePath = filename;

    ui->statusBar->showMessage("loading Image...");

    input = QImage(filename);

    //enable gui buttons
    ui->pushButton_calc->setEnabled(true);
    ui->label_display->setEnabled(true);
    ui->radioButton_input->setEnabled(true);

    //switch to input preview
    ui->radioButton_input->setChecked(true);

    preview();
    ui->statusBar->clearMessage();
}

void MainWindow::calc() {
    ui->statusBar->showMessage("calculating normalmap...");

    double strength = ui->doubleSpinBox_strength->value();
    bool invert = ui->checkBox_invertHeight->isChecked();
    bool tileable = ui->checkBox_tileable->isChecked();

    if(ui->comboBox->currentIndex() == 0) {
        //sobel operator
        Sobel sobelGenerator;
        result = sobelGenerator.calculateNormalmap(input, strength, invert, tileable);
    }

    //enable gui buttons
    ui->pushButton_save->setEnabled(true);
    ui->radioButton_generated->setEnabled(true);
    //switch preview buttons from input to generated
    ui->radioButton_generated->setChecked(true);

    preview();
    ui->statusBar->clearMessage();
}

void MainWindow::save() {
    QString filename = QFileDialog::getSaveFileName(this, "Save as", loadedImagePath,
                                                    "Image Formats (*.png *.jpg *.jpeg *.tiff *.ppm *.bmp *.xpm)");

    if(filename.isEmpty())
        return;

    QFileInfo file(filename);
    if(!file.baseName().isEmpty() && file.suffix().isEmpty())
        filename += ".png";

    if(!result.save(filename))
        QMessageBox::information(this, "Error while saving image", "Image not saved!");
    else
        ui->statusBar->showMessage("image saved", 2000);
}

void MainWindow::preview() {
    ui->graphicsView->scene()->clear();

    if(ui->radioButton_generated->isChecked()) {
        ui->graphicsView->scene()->addPixmap(QPixmap::fromImage(result));
    }
    else {
        ui->graphicsView->scene()->addPixmap(QPixmap::fromImage(input));
    }
}

void MainWindow::zoomIn() {
    ui->graphicsView->scale(1.2, 1.2);
}

void MainWindow::zoomOut() {
    ui->graphicsView->scale(0.8, 0.8);
}

void MainWindow::resetZoom() {
    //todo
}

void MainWindow::fitInView() {
    ui->graphicsView->fitInView(ui->graphicsView->scene()->sceneRect(), Qt::KeepAspectRatio);
}
