#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //connect Signals of GUI elements with Slots of this class
    connectSignalSlots();

    //initialize graphicsview
    QGraphicsScene *scene = new QGraphicsScene();
    ui->graphicsView->setScene(scene);
    scene->setBackgroundBrush(QBrush(Qt::darkGray));
    ui->graphicsView->setRenderHints(QPainter::HighQualityAntialiasing | QPainter::SmoothPixmapTransform);

    //initialize QImage objects to store the calculated maps
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
    QString filename = QFileDialog::getOpenFileName(this,
                                                    "Open Image File",
                                                    QDir::homePath(),
                                                    "Image Formats (*.png *.jpg *.jpeg *.tiff *.ppm *.bmp *.xpm)");
    if(filename.isEmpty())
        return;
    //store the path the image was loaded from (for saving later)
    loadedImagePath = filename;

    ui->statusBar->showMessage("loading Image...");

    //load the image
    input = QImage(filename);

    //enable ui buttons
    ui->pushButton_calcNormal->setEnabled(true);
    ui->pushButton_calcSpec->setEnabled(true);
    ui->pushButton_calcDisplace->setEnabled(true);
    //switch active tab to input
    ui->tabWidget->setCurrentIndex(0);

    //clear all previously generated images
    channelIntensity = QImage();
    normalmap = QImage();
    specmap = QImage();
    displacementmap = QImage();

    //display single image channels if the option was already chosen
    if(ui->checkBox_displayChannelIntensity->isChecked())
        displayChannelIntensity();
    else
        preview(0);

    ui->statusBar->clearMessage();
}

void MainWindow::calcNormal() {
    ui->statusBar->showMessage("calculating normalmap...");

    //timer for measuring calculation time
    QElapsedTimer timer;
    timer.start();

    //normalmap parameters
    double strength = ui->doubleSpinBox_strength->value();
    bool invert = ui->checkBox_invertHeight->isChecked();
    bool tileable = ui->checkBox_tileable->isChecked();

    //color channel mode
    IntensityMap::Mode mode = IntensityMap::AVERAGE;
    if(ui->comboBox_mode_normal->currentIndex() == 0)
        mode = IntensityMap::AVERAGE;
    else if(ui->comboBox_mode_normal->currentIndex() == 1)
        mode = IntensityMap::MAX;

    //color channels to use
    bool useRed = ui->checkBox_useRed_normal->isChecked();
    bool useGreen = ui->checkBox_useGreen_normal->isChecked();
    bool useBlue = ui->checkBox_useBlue_normal->isChecked();
    bool useAlpha = ui->checkBox_useAlpha_normal->isChecked();

    //kernel to use
    NormalmapGenerator::Kernel kernel = NormalmapGenerator::SOBEL;
    if(ui->comboBox_method->currentIndex() == 0)
        kernel = NormalmapGenerator::SOBEL;
    else if(ui->comboBox_method->currentIndex() == 1)
        kernel = NormalmapGenerator::PREWITT;

    //setup generator and calculate map
    NormalmapGenerator normalmapGenerator(mode, useRed, useGreen, useBlue, useAlpha);
    normalmap = normalmapGenerator.calculateNormalmap(input, kernel, strength, invert, tileable);

    //display time it took to calculate the map
    this->lastCalctime_normal = timer.elapsed();
    displayCalcTime(lastCalctime_normal, "normalmap", 5000);
    /*
    std::cout << "normalmap calculated, it took " << lastCalctime_normal << "ms" << std::endl;
    ui->statusBar->clearMessage();
    QString msg = generateElapsedTimeMsg(lastCalctime_normal, "normalmap");
    ui->statusBar->showMessage(msg, 5000);
    ui->label_lastCalcTime->setText("(Last Calc. Time: " + QString::number((double)lastCalctime_normal / 1000.0) + "s)");
    */

    //enable ui buttons
    ui->pushButton_save->setEnabled(true);

    //preview in normalmap tab
    preview(1);
}

void MainWindow::calcSpec() {
    ui->statusBar->showMessage("calculating specularmap...");

    //timer for measuring calculation time
    QElapsedTimer timer;
    timer.start();

    //color channel mode
    IntensityMap::Mode mode = IntensityMap::AVERAGE;
    if(ui->comboBox_mode_spec->currentIndex() == 0)
        mode = IntensityMap::AVERAGE;
    else if(ui->comboBox_mode_spec->currentIndex() == 1)
        mode = IntensityMap::MAX;

    //color channel multipliers to use
    double redMultiplier = ui->doubleSpinBox_spec_redMul->value();
    double greenMultiplier = ui->doubleSpinBox_spec_greenMul->value();
    double blueMultiplier = ui->doubleSpinBox_spec_blueMul->value();
    double alphaMultiplier = ui->doubleSpinBox_spec_alphaMul->value();
    double scale = ui->doubleSpinBox_spec_scale->value();

    //setup generator and calculate map
    SpecularmapGenerator specularmapGenerator(mode, redMultiplier, greenMultiplier, blueMultiplier, alphaMultiplier);
    specmap = specularmapGenerator.calculateSpecmap(input, scale);

    //display time it took to calculate the map
    this->lastCalctime_specular = timer.elapsed();
    displayCalcTime(lastCalctime_specular, "specularmap", 5000);

    //enable ui buttons
    ui->pushButton_save->setEnabled(true);

    //preview in specular map tab
    preview(2);
}

void MainWindow::calcDisplace() {
    //todo
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
    QString filename = QFileDialog::getSaveFileName(this,
                                                    "Save as",
                                                    loadedImagePath,
                                                    "Image Formats (*.png *.jpg *.jpeg *.tiff *.ppm *.bmp *.xpm)");
    //if saving process was aborted
    if(filename.isEmpty())
        return;

    //if no file suffix was chosen, automatically use the PNG format
    QFileInfo file(filename);
    if(!file.baseName().isEmpty() && file.suffix().isEmpty())
        filename += ".png";

    //append a suffix to the map names (result: path/original_normal.png)
    QString name_normal = file.absolutePath() + "/" + file.baseName() + "_normal." + file.suffix();
    QString name_specular = file.absolutePath() + "/" + file.baseName() + "_spec." + file.suffix();
    QString name_displace = file.absolutePath() + "/" + file.baseName() + "_displace." + file.suffix();

    //check if maps where generated, if yes, check if it could be saved
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

    //store export path
    exportPath = file.absolutePath();
    //enable "Open Export Folder" gui button
    ui->pushButton_openExportFolder->setEnabled(true);
}

//overloaded version of preview that chooses the map to preview automatically
void MainWindow::preview() {
    preview(ui->tabWidget->currentIndex());
}

//preview the map in the selected tab
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

//resets zoom to 1:1
void MainWindow::resetZoom() {
    ui->graphicsView->resetTransform();
}

//fits the preview into the graphicsView
void MainWindow::fitInView() {
   ui->graphicsView->scene()->setSceneRect(QRectF(0, 0, input.width(), input.height()));
   ui->graphicsView->setSceneRect(ui->graphicsView->scene()->sceneRect());
   ui->graphicsView->fitInView(ui->graphicsView->scene()->sceneRect(), Qt::KeepAspectRatio);
}

//displays single color channels of the image (handled by an intensitymap)
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
//this Slot is for parameter input fields/buttons in the gui
void MainWindow::autoUpdate() {
    int autoUpdateThreshold_ms = ui->doubleSpinBox_autoUpdateThreshold->value() * 1000.0;

    switch(ui->tabWidget->currentIndex()) {
    case 0:
        break;
    case 1:
        if(lastCalctime_normal < autoUpdateThreshold_ms)
            calcNormal();
        break;
    case 2:
        if(lastCalctime_specular < autoUpdateThreshold_ms)
            calcSpec();
        break;
    case 3:
        if(lastCalctime_displace < autoUpdateThreshold_ms)
            calcDisplace();
        break;
    default:
        break;
    }
}

//generate a message that shows the elapsed time of a calculation process
//example output: "calculated normalmap (1.542 seconds)"
QString MainWindow::generateElapsedTimeMsg(int calcTimeMs, QString mapType) {
    double calcTimeS = (double)calcTimeMs / 1000.0;

    QString elapsedTimeMsg("calculated ");
    elapsedTimeMsg.append(mapType);
    elapsedTimeMsg.append(" (");
    elapsedTimeMsg.append(QString::number(calcTimeS));
    elapsedTimeMsg.append(" seconds)");
    return elapsedTimeMsg;
}

void MainWindow::openExportFolder() {
    QDesktopServices::openUrl(QUrl(exportPath));
}

void MainWindow::displayCalcTime(int calcTime_ms, QString mapType, int duration_ms) {
    std::cout << mapType.toStdString() << " calculated, it took " << calcTime_ms << "ms" << std::endl;
    ui->statusBar->clearMessage();
    QString msg = generateElapsedTimeMsg(calcTime_ms, mapType);
    ui->statusBar->showMessage(msg, duration_ms);
    ui->label_lastCalcTime->setText("(Last Calc. Time: " + QString::number((double)calcTime_ms / 1000.0) + "s)");
}

//connects gui buttons with Slots in this class
void MainWindow::connectSignalSlots() {
    //connect signals/slots
    //load/save/open export folder
    connect(ui->pushButton_load, SIGNAL(clicked()), this, SLOT(load()));
    connect(ui->pushButton_save, SIGNAL(clicked()), this, SLOT(saveAll()));
    connect(ui->pushButton_openExportFolder, SIGNAL(clicked()), this, SLOT(openExportFolder()));
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
}
