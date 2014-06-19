#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "graphicsscene.h"

#include <QTreeView>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //connect Signals of GUI elements with Slots of this class
    connectSignalSlots();

    //initialize graphicsview
    GraphicsScene *scene = new GraphicsScene();
    ui->graphicsView->setScene(scene);
    scene->setBackgroundBrush(QBrush(Qt::darkGray));
    scene->addText("Start by dragging images here.");
    ui->graphicsView->setRenderHints(QPainter::HighQualityAntialiasing | QPainter::SmoothPixmapTransform);
    ui->graphicsView->setAcceptDrops(true);

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

    //initialize stopQueue flag
    stopQueue = false;

    //show default status message
    ui->statusBar->showMessage("Drag images into the empty preview window to load them.");
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::loadSingleDropped(QUrl url) {
    if(load(url))
        addImageToQueue(url);
}

void MainWindow::loadMultipleDropped(QList<QUrl> urls) {
    //test if image formats are supported
    bool containedInvalidFormat = false;
    bool loadedFirstValidImage = false;

    for(int i = 0; i < urls.size(); i++) {
        QString suffix = QFileInfo(urls.at(i).fileName()).suffix().toLower();
        QString supported = "png jpg jpeg tiff.ppm bmp xpm";

        if(supported.contains(suffix)) {
            //image format is supported, add to queue
            addImageToQueue(urls.at(i));
            //if it is the first valid image, load and preview it
            if(!loadedFirstValidImage) {
                load(urls.at(i));
                loadedFirstValidImage = true;
            }
        }
        else {
            containedInvalidFormat = true;
        }
    }

    if(containedInvalidFormat)
        QMessageBox::information(this, "Not All Images Loaded Into Queue",
                                 "Some images had unsupported formats and where not loaded into the queue!");
}

//load the image specified in the url
bool MainWindow::load(QUrl url) {
    if(!url.isValid()) {
        throw "[load] invalid url!";
        return false;
    }

    ui->statusBar->showMessage("loading Image: " + url.fileName());

    //load the image
    input = QImage(url.toLocalFile());

    if(input.isNull()) {
        ui->statusBar->showMessage("Error: Image " + url.fileName() + " NOT loaded!", 5000);
        QMessageBox::information(this, "Error while loading image",
                                 "Image not loaded!\nMost likely the image format is not supported.");
        return false;
    }

    //store the path the image was loaded from (for saving later)
    if(exportPath.isEmpty())
        exportPath = url.adjusted(QUrl::RemoveFilename);
    loadedImagePath = url;

    //enable ui buttons
    ui->pushButton_calcNormal->setEnabled(true);
    ui->pushButton_calcSpec->setEnabled(true);
    ui->pushButton_calcDisplace->setEnabled(true);
    ui->checkBox_displayChannelIntensity->setEnabled(true);
    enableAutoupdate(true);
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

    return true;
}

//load images using the file dialog
void MainWindow::loadUserFilePath() {
    QList<QUrl> urls = QFileDialog::getOpenFileUrls(this,
                                                     "Open Image File",
                                                     QDir::homePath(),
                                                     "Image Formats (*.png *.jpg *.jpeg *.tiff *.ppm *.bmp *.xpm)");
    loadMultipleDropped(urls);
}

void MainWindow::calcNormal() {
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
}

void MainWindow::calcSpec() {
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
}

void MainWindow::calcDisplace() {
    //todo
}


void MainWindow::calcNormalAndPreview() {
    ui->statusBar->showMessage("calculating normalmap...");

    //timer for measuring calculation time
    QElapsedTimer timer;
    timer.start();

    //calculate map
    calcNormal();

    //display time it took to calculate the map
    this->lastCalctime_normal = timer.elapsed();
    displayCalcTime(lastCalctime_normal, "normalmap", 5000);

    //enable ui buttons
    ui->pushButton_save->setEnabled(true);

    //preview in normalmap tab
    preview(1);
}

void MainWindow::calcSpecAndPreview() {
    ui->statusBar->showMessage("calculating specularmap...");

    //timer for measuring calculation time
    QElapsedTimer timer;
    timer.start();

    //calculate map
    calcSpec();

    //display time it took to calculate the map
    this->lastCalctime_specular = timer.elapsed();
    displayCalcTime(lastCalctime_specular, "specularmap", 5000);

    //enable ui buttons
    ui->pushButton_save->setEnabled(true);

    //preview in specular map tab
    preview(2);
}

void MainWindow::calcDisplaceAndPreview() {
    //todo
}

void MainWindow::processQueue() {
    if(ui->listWidget_queue->count() == 0)
        return;

    if(!exportPath.isValid()) {
        QMessageBox::information(this, "Invalid Export Path", "Export path is invalid!");
        return;
    }

    //enable stop button
    ui->pushButton_stopProcessingQueue->setEnabled(true);

    double percentageBase = 100.0 / ui->listWidget_queue->count();

    for(int i = 0; i < ui->listWidget_queue->count() && !stopQueue; i++)
    {
        QueueItem *item = (QueueItem*)(ui->listWidget_queue->item(i));

        //display status
        ui->statusBar->showMessage("Processing Queue Item: " + item->text());
        ui->progressBar_Queue->setValue((int)(percentageBase * (i + 1)));
        ui->listWidget_queue->item(i)->setSelected(true);

        //load image
        load(item->getUrl());

        //calculate maps
        if(ui->checkBox_queue_generateNormal->isChecked())
            calcNormal();
        if(ui->checkBox_queue_generateSpec->isChecked())
            calcSpec();
        if(ui->checkBox_queue_generateDisplace->isChecked())
            calcDisplace();

        //save maps
        QUrl exportUrl = QUrl::fromLocalFile(exportPath.toLocalFile() + "/" + item->text());
        std::cout << "[Queue] Image " << i + 1 << " exported: "
                  << exportUrl.toLocalFile().toStdString() << std::endl;
        save(exportUrl);

        //user interface should stay responsive
        QCoreApplication::processEvents();
    }

    //disable stop button
    ui->pushButton_stopProcessingQueue->setEnabled(false);
    stopQueue = false;

    //enable "Open Export Folder" gui button
    ui->pushButton_openExportFolder->setEnabled(true);
}

//tell the queue to stop processing
void MainWindow::stopProcessingQueue() {
    stopQueue = true;
}

//save maps using the file dialog
void MainWindow::saveUserFilePath() {
    QUrl url = QFileDialog::getSaveFileUrl(this,
                                           "Save as",
                                           loadedImagePath,
                                           "Image Formats (*.png *.jpg *.jpeg *.tiff *.ppm *.bmp *.xpm)");
    save(url);
}

void MainWindow::save(QUrl url) {
    //if saving process was aborted
    if(!url.isValid())
        return;

    QString path = url.toLocalFile();

    //if no file suffix was chosen, automatically use the PNG format
    QFileInfo file(path);
    if(!file.baseName().isEmpty() && file.suffix().isEmpty())
        path += ".png";

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
    exportPath = url.adjusted(QUrl::RemoveFilename);
    //enable "Open Export Folder" gui button
    ui->pushButton_openExportFolder->setEnabled(true);
}

//change the path the queue exports the maps to
void MainWindow::changeOutputPathQueue() {
    QUrl startUrl = QDir::homePath();
    if(exportPath.isValid())
        startUrl = exportPath;

    exportPath = QFileDialog::getExistingDirectoryUrl(this,
                                                           "Choose Export Folder",
                                                           startUrl);
    std::cout << "export path changed to: " << exportPath.toLocalFile().toStdString() << std::endl;
}

//enable/disable custom output path button
void MainWindow::updateQueueExportOptions() {
    ui->pushButton_changeOutputPath_Queue->setEnabled(ui->radioButton_exportUserDefined->isChecked());
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
        if(!input.isNull() && normalmap.isNull() && input.width() < 3000 && input.height() < 3000) {
            //if an image was loaded and a normalmap was not yet generated and the image is not too large
            //automatically generate the normalmap
            calcNormal();
        }
        ui->graphicsView->scene()->addPixmap(QPixmap::fromImage(normalmap));
        break;
    case 2:
        //spec
        if(!input.isNull() && specmap.isNull() && input.width() < 3000 && input.height() < 3000) {
            //if an image was loaded and a specmap was not yet generated and the image is not too large
            //automatically generate the specmap
            calcSpec();
        }
        ui->graphicsView->scene()->addPixmap(QPixmap::fromImage(specmap));
        break;
    case 3:
        //displacement
        if(!input.isNull() && displacementmap.isNull() && input.width() < 3000 && input.height() < 3000) {
            //if an image was loaded and a dispmap was not yet generated and the image is not too large
            //automatically generate the displacementmap
            calcDisplace();
        }
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
    if(!ui->checkBox_autoUpdate->isChecked() || !ui->checkBox_autoUpdate->isEnabled())
        return;

    int autoUpdateThreshold_ms = ui->doubleSpinBox_autoUpdateThreshold->value() * 1000.0;

    switch(ui->tabWidget->currentIndex()) {
    case 0:
        break;
    case 1:
        if(lastCalctime_normal < autoUpdateThreshold_ms)
            calcNormalAndPreview();
        break;
    case 2:
        if(lastCalctime_specular < autoUpdateThreshold_ms)
            calcSpecAndPreview();
        break;
    case 3:
        if(lastCalctime_displace < autoUpdateThreshold_ms)
            calcDisplaceAndPreview();
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
    QDesktopServices::openUrl(exportPath);
}

//display the last calculation time in the statusbar
void MainWindow::displayCalcTime(int calcTime_ms, QString mapType, int duration_ms) {
    std::cout << mapType.toStdString() << " for item " << loadedImagePath.fileName().toStdString()
              << " calculated, it took " << calcTime_ms << "ms" << std::endl;
    ui->statusBar->clearMessage();
    QString msg = generateElapsedTimeMsg(calcTime_ms, mapType);
    ui->statusBar->showMessage(msg, duration_ms);
    ui->label_autoUpdate_lastCalcTime->setText("(Last Calc. Time: " + QString::number((double)calcTime_ms / 1000.0) + "s)");
}

void MainWindow::enableAutoupdate(bool on) {
    ui->checkBox_autoUpdate->setEnabled(on);
    ui->label_autoUpdate_lastCalcTime->setEnabled(on);
    ui->label_autoUpdate_text->setEnabled(on);
    ui->doubleSpinBox_autoUpdateThreshold->setEnabled(on);
}

//add single image to queue
void MainWindow::addImageToQueue(QUrl url) {
    QueueItem *item = new QueueItem(url, url.fileName(), ui->listWidget_queue, 0);
    ui->listWidget_queue->addItem(item);
}

//add multiple images to queue
void MainWindow::addImageToQueue(QList<QUrl> urls) {
    for(int i = 0; i < urls.size(); i++) {
        addImageToQueue(urls.at(i));
    }
}

void MainWindow::removeImagesFromQueue() {
    qDeleteAll(ui->listWidget_queue->selectedItems());
}

void MainWindow::queueItemDoubleClicked(QListWidgetItem* item) {
    //load image that was doubleclicked
    load(((QueueItem*)item)->getUrl());
}

//connects gui buttons with Slots in this class
void MainWindow::connectSignalSlots() {
    //connect signals/slots
    //load/save/open export folder
    connect(ui->pushButton_load, SIGNAL(clicked()), this, SLOT(loadUserFilePath()));
    connect(ui->pushButton_save, SIGNAL(clicked()), this, SLOT(saveUserFilePath()));
    connect(ui->pushButton_openExportFolder, SIGNAL(clicked()), this, SLOT(openExportFolder()));
    //zoom
    connect(ui->pushButton_zoomIn, SIGNAL(clicked()), this, SLOT(zoomIn()));
    connect(ui->pushButton_zoomOut, SIGNAL(clicked()), this, SLOT(zoomOut()));
    connect(ui->pushButton_resetZoom, SIGNAL(clicked()), this, SLOT(resetZoom()));
    connect(ui->pushButton_fitInView, SIGNAL(clicked()), this, SLOT(fitInView()));
    //calculate
    connect(ui->pushButton_calcNormal, SIGNAL(clicked()), this, SLOT(calcNormalAndPreview()));
    connect(ui->pushButton_calcSpec, SIGNAL(clicked()), this, SLOT(calcSpecAndPreview()));
    connect(ui->pushButton_calcDisplace, SIGNAL(clicked()), this, SLOT(calcDisplaceAndPreview()));
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
    //graphicsview drag and drop
    connect(ui->graphicsView, SIGNAL(singleImageDropped(QUrl)), this, SLOT(loadSingleDropped(QUrl)));
    connect(ui->graphicsView, SIGNAL(multipleImagesDropped(QList<QUrl>)), this, SLOT(loadMultipleDropped(QList<QUrl>)));
    //queue (item widget)
    connect(ui->pushButton_removeImagesFromQueue, SIGNAL(clicked()), this, SLOT(removeImagesFromQueue()));
    connect(ui->pushButton_processQueue, SIGNAL(clicked()), this, SLOT(processQueue()));
    connect(ui->pushButton_stopProcessingQueue, SIGNAL(clicked()), this, SLOT(stopProcessingQueue()));
    connect(ui->pushButton_changeOutputPath_Queue, SIGNAL(clicked()), this, SLOT(changeOutputPathQueue()));
    connect(ui->buttonGroup_exportFolder, SIGNAL(buttonClicked(int)), this, SLOT(updateQueueExportOptions()));
    connect(ui->listWidget_queue, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(queueItemDoubleClicked(QListWidgetItem*)));
}
