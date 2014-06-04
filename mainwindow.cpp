#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //connect signals/slots
    //load/save
    connect(ui->pushButton_load, SIGNAL(clicked()), this, SLOT(load()));
    connect(ui->pushButton_save, SIGNAL(clicked()), this, SLOT(saveAll()));
    //zoom
    connect(ui->pushButton_zoomIn, SIGNAL(clicked()), this, SLOT(zoomIn()));
    connect(ui->pushButton_zoomOut, SIGNAL(clicked()), this, SLOT(zoomOut()));
    connect(ui->pushButton_resetZoom, SIGNAL(clicked()), this, SLOT(resetZoom()));
    connect(ui->pushButton_fitInView, SIGNAL(clicked()), this, SLOT(fitInView()));
    //calculate
    connect(ui->pushButton_calcNormal, SIGNAL(clicked()), this, SLOT(calcNormal()));
    connect(ui->pushButton_calcSpec, SIGNAL(clicked()), this, SLOT(calcSpec()));
    connect(ui->pushButton_calcDisplace, SIGNAL(clicked()), this, SLOT(calcDisplace()));
    //switch between tabs
    connect(ui->tabWidget, SIGNAL(tabBarClicked(int)), this, SLOT(preview(int)));
    //display channel intensity
    connect(ui->checkBox_displayChannelIntensity, SIGNAL(clicked(bool)), this, SLOT(preview()));
    connect(ui->checkBox_displayChannelIntensity, SIGNAL(clicked(bool)), ui->radioButton_displayRed, SLOT(setEnabled(bool)));
    connect(ui->checkBox_displayChannelIntensity, SIGNAL(clicked(bool)), ui->radioButton_displayGreen, SLOT(setEnabled(bool)));
    connect(ui->checkBox_displayChannelIntensity, SIGNAL(clicked(bool)), ui->radioButton_displayBlue, SLOT(setEnabled(bool)));
    connect(ui->checkBox_displayChannelIntensity, SIGNAL(clicked(bool)), ui->radioButton_displayAlpha, SLOT(setEnabled(bool)));

    connect(ui->radioButton_displayRed, SIGNAL(clicked()), this, SLOT(displayChannelIntensity()));
    connect(ui->radioButton_displayGreen, SIGNAL(clicked()), this, SLOT(displayChannelIntensity()));
    connect(ui->radioButton_displayBlue, SIGNAL(clicked()), this, SLOT(displayChannelIntensity()));
    connect(ui->radioButton_displayAlpha, SIGNAL(clicked()), this, SLOT(displayChannelIntensity()));
    connect(ui->checkBox_displayChannelIntensity, SIGNAL(clicked()), this, SLOT(displayChannelIntensity()));
    //autoupdate after changed values
    // spec autoupdate
    connect(ui->doubleSpinBox_spec_redMul, SIGNAL(valueChanged(double)), this, SLOT(autoUpdate()));
    connect(ui->doubleSpinBox_spec_greenMul, SIGNAL(valueChanged(double)), this, SLOT(autoUpdate()));
    connect(ui->doubleSpinBox_spec_blueMul, SIGNAL(valueChanged(double)), this, SLOT(autoUpdate()));
    connect(ui->doubleSpinBox_spec_alphaMul, SIGNAL(valueChanged(double)), this, SLOT(autoUpdate()));
    connect(ui->doubleSpinBox_spec_scale, SIGNAL(valueChanged(double)), this, SLOT(autoUpdate()));
    connect(ui->comboBox_mode_spec, SIGNAL(currentIndexChanged(int)), this, SLOT(autoUpdate()));
    // normal autoupdate
    connect(ui->checkBox_useRed_normal, SIGNAL(clicked()), this, SLOT(autoUpdate()));
    connect(ui->checkBox_useGreen_normal, SIGNAL(clicked()), this, SLOT(autoUpdate()));
    connect(ui->checkBox_useBlue_normal, SIGNAL(clicked()), this, SLOT(autoUpdate()));
    connect(ui->checkBox_useAlpha_normal, SIGNAL(clicked()), this, SLOT(autoUpdate()));
    connect(ui->comboBox_mode_normal, SIGNAL(currentIndexChanged(int)), this, SLOT(autoUpdate()));
    connect(ui->comboBox_method, SIGNAL(currentIndexChanged(int)), this, SLOT(autoUpdate()));
    connect(ui->doubleSpinBox_strength, SIGNAL(valueChanged(double)), this, SLOT(autoUpdate()));
    connect(ui->checkBox_tileable, SIGNAL(clicked()), this, SLOT(autoUpdate()));
    connect(ui->checkBox_invertHeight, SIGNAL(clicked()), this, SLOT(autoUpdate()));

    //initialize graphicsview
    QGraphicsScene *scene = new QGraphicsScene();
    ui->graphicsView->setScene(scene);
    scene->setBackgroundBrush(QBrush(Qt::darkGray));

    //initialize QImage objects
    input = QImage();
    channelIntensity = QImage();
    normalmap = QImage();
    specmap = QImage();
    displacementmap = QImage();

    //initialize calctimes
    lastCalctime_normal = 0;
    lastCalctime_specular = 0;
    lastCalctime_displace = 0;
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

    //enable ui buttons
    ui->pushButton_calcNormal->setEnabled(true);
    ui->pushButton_calcSpec->setEnabled(true);
    ui->pushButton_calcDisplace->setEnabled(true);
    //switch active tab to input
    ui->tabWidget->setCurrentIndex(0);

    //clear all generated images
    channelIntensity = QImage();
    normalmap = QImage();
    specmap = QImage();
    displacementmap = QImage();

    if(ui->checkBox_displayChannelIntensity->isChecked())
        displayChannelIntensity();

    preview(0);
    ui->statusBar->clearMessage();
}

void MainWindow::calcNormal() {
    ui->statusBar->showMessage("calculating normalmap...");

    QElapsedTimer timer;
    timer.start();

    double strength = ui->doubleSpinBox_strength->value();
    bool invert = ui->checkBox_invertHeight->isChecked();
    bool tileable = ui->checkBox_tileable->isChecked();

    IntensityMap::Mode mode = IntensityMap::AVERAGE;
    if(ui->comboBox_mode_normal->currentIndex() == 0)
        mode = IntensityMap::AVERAGE;
    else if(ui->comboBox_mode_normal->currentIndex() == 1)
        mode = IntensityMap::MAX;

    bool useRed = ui->checkBox_useRed_normal->isChecked();
    bool useGreen = ui->checkBox_useGreen_normal->isChecked();
    bool useBlue = ui->checkBox_useBlue_normal->isChecked();
    bool useAlpha = ui->checkBox_useAlpha_normal->isChecked();

    NormalmapGenerator::Kernel kernel = NormalmapGenerator::SOBEL;
    if(ui->comboBox_method->currentIndex() == 0) {
        //use sobel operator
        kernel = NormalmapGenerator::SOBEL;
    }

    NormalmapGenerator normalmapGenerator(mode, useRed, useGreen, useBlue, useAlpha);
    normalmap = normalmapGenerator.calculateNormalmap(input, kernel, strength, invert, tileable);

    this->lastCalctime_normal = timer.elapsed();
    std::cout << "normalmap calculated, it took " << lastCalctime_normal << "ms" << std::endl;

    //enable ui buttons
    ui->pushButton_save->setEnabled(true);

    preview(1);
    ui->statusBar->clearMessage();
    QString msg = generateElapsedTimeMsg(lastCalctime_normal, "normalmap");
    ui->statusBar->showMessage(msg, 5000);
}

void MainWindow::calcSpec() {
    ui->statusBar->showMessage("calculating specularmap...");

    QElapsedTimer timer;
    timer.start();

    IntensityMap::Mode mode = IntensityMap::AVERAGE;
    if(ui->comboBox_mode_spec->currentIndex() == 0)
        mode = IntensityMap::AVERAGE;
    else if(ui->comboBox_mode_spec->currentIndex() == 1)
        mode = IntensityMap::MAX;

    double redMultiplier = ui->doubleSpinBox_spec_redMul->value();
    double greenMultiplier = ui->doubleSpinBox_spec_greenMul->value();
    double blueMultiplier = ui->doubleSpinBox_spec_blueMul->value();
    double alphaMultiplier = ui->doubleSpinBox_spec_alphaMul->value();
    double scale = ui->doubleSpinBox_spec_scale->value();

    //calculate specmap
    SpecularmapGenerator specularmapGenerator(mode, redMultiplier, greenMultiplier, blueMultiplier, alphaMultiplier);
    specmap = specularmapGenerator.calculateSpecmap(input, scale);

    this->lastCalctime_specular = timer.elapsed();
    std::cout << "specmap calculated, it took " << lastCalctime_specular << "ms" << std::endl;

    //enable ui buttons
    ui->pushButton_save->setEnabled(true);

    preview(2);
    ui->statusBar->clearMessage();
    QString msg = generateElapsedTimeMsg(lastCalctime_normal, "specularmap");
    ui->statusBar->showMessage(msg, 5000);
}

void MainWindow::calcDisplace() {

}

//deprecated
void MainWindow::save() {
    QString filename = QFileDialog::getSaveFileName(this, "Save as", loadedImagePath,
                                                    "Image Formats (*.png *.jpg *.jpeg *.tiff *.ppm *.bmp *.xpm)");

    if(filename.isEmpty())
        return;

    QFileInfo file(filename);
    if(!file.baseName().isEmpty() && file.suffix().isEmpty())
        filename += ".png";

    if(!normalmap.save(filename))
        QMessageBox::information(this, "Error while saving image", "Image not saved!");
    else
        ui->statusBar->showMessage("image saved", 2000);
}

void MainWindow::saveAll() {
    QString filename = QFileDialog::getSaveFileName(this, "Save as", loadedImagePath,
                                                    "Image Formats (*.png *.jpg *.jpeg *.tiff *.ppm *.bmp *.xpm)");

    if(filename.isEmpty())
        return;

    QFileInfo file(filename);
    if(!file.baseName().isEmpty() && file.suffix().isEmpty())
        filename += ".png";

    QString name_normal = file.absolutePath() + "/" + file.baseName() + "_normal." + file.suffix();
    QString name_specular = file.absolutePath() + "/" + file.baseName() + "_spec." + file.suffix();
    QString name_displace = file.absolutePath() + "/" + file.baseName() + "_displace." + file.suffix();

    if(!normalmap.isNull()) {
        if(!normalmap.save(name_normal))
            QMessageBox::information(this, "Error while saving Normalmap", "Normalmap not saved!");
        else
            ui->statusBar->showMessage("Normalmap saved as \"" + name_normal + "\"", 4000);
    }

    if(!specmap.isNull()) {
        if(!specmap.save(name_specular))
            QMessageBox::information(this, "Error while saving Specularmap", "Specularmap not saved!");
        else
            ui->statusBar->showMessage("Specularmap saved as \"" + name_specular + "\"", 4000);
    }

    if(!displacementmap.isNull()) {
        if(!displacementmap.save(name_displace))
            QMessageBox::information(this, "Error while saving Displacementmap", "Displacementmap not saved!");
        else
            ui->statusBar->showMessage("Displacementmap saved as \"" + name_displace + "\"", 4000);
    }
}

void MainWindow::preview() {
    preview(ui->tabWidget->currentIndex());
}

void MainWindow::preview(int tab) {
    ui->graphicsView->scene()->clear();

    switch(tab) {
    case 0:
        //input
        if(ui->checkBox_displayChannelIntensity->isChecked() && !input.isNull()) {
            ui->graphicsView->scene()->addPixmap(QPixmap::fromImage(channelIntensity));
        }
        else {
            ui->graphicsView->scene()->addPixmap(QPixmap::fromImage(input));
        }
        break;
    case 1:
        //normal
        ui->graphicsView->scene()->addPixmap(QPixmap::fromImage(normalmap));
        break;
    case 2:
        //spec
        ui->graphicsView->scene()->addPixmap(QPixmap::fromImage(specmap));
        break;
    case 3:
        //displacement
        ui->graphicsView->scene()->addPixmap(QPixmap::fromImage(displacementmap));
        break;
    }
}

void MainWindow::zoomIn() {
    ui->graphicsView->scale(1.2, 1.2);
}

void MainWindow::zoomOut() {
    ui->graphicsView->scale(0.8, 0.8);
}

void MainWindow::resetZoom() {
    ui->graphicsView->resetTransform();
}

void MainWindow::fitInView() {
   ui->graphicsView->scene()->setSceneRect(QRectF(0, 0, input.width(), input.height()));
   ui->graphicsView->setSceneRect(ui->graphicsView->scene()->sceneRect());
   ui->graphicsView->fitInView(ui->graphicsView->scene()->sceneRect(), Qt::KeepAspectRatio);
}

void MainWindow::displayChannelIntensity() {
    IntensityMap temp;
    if(ui->radioButton_displayRed->isChecked())
        temp = IntensityMap(input, IntensityMap::AVERAGE, true, false, false, false);
    else if(ui->radioButton_displayGreen->isChecked())
        temp = IntensityMap(input, IntensityMap::AVERAGE, false, true, false, false);
    else if(ui->radioButton_displayBlue->isChecked())
        temp = IntensityMap(input, IntensityMap::AVERAGE, false, false, true, false);
    else
        temp = IntensityMap(input, IntensityMap::AVERAGE, false, false, false, true);

    this->channelIntensity = temp.convertToQImage();
    preview(0);
}

//automatically update the preview if the calculation took only a certain amount of time
//in milliseconds, e.g. 500 (0.5 seconds)
void MainWindow::autoUpdate() {
    switch(ui->tabWidget->currentIndex()) {
    case 0:
        break;
    case 1:
        if(lastCalctime_normal < 500)
            calcNormal();
        break;
    case 2:
        if(lastCalctime_specular < 500)
            calcSpec();
        break;
    case 3:
        if(lastCalctime_displace < 500)
            calcDisplace();
        break;
    default:
        break;
    }
}

QString MainWindow::generateElapsedTimeMsg(int calcTimeMs, QString mapType) {
    double calcTimeS = (double)calcTimeMs / 1000.0;

    QString elapsedTimeMsg("calculated ");
    elapsedTimeMsg.append(mapType);
    elapsedTimeMsg.append(" (");
    elapsedTimeMsg.append(QString::number(calcTimeS));
    elapsedTimeMsg.append(" seconds)");
    return elapsedTimeMsg;
}
