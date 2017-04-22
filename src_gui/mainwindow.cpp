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

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "graphicsscene.h"
#include "aboutdialog.h"
#include "src_generators/normalmapgenerator.h"
#include "src_generators/specularmapgenerator.h"
#include "src_generators/ssaogenerator.h"
#include "src_generators/intensitymap.h"
#include "src_generators/gaussianblur.h"

#include <QMessageBox>
#include <QFileDialog>
#include <QElapsedTimer>
#include <QDesktopServices>
#include <QTreeView>
#include <QGraphicsPixmapItem>
#include <QSettings>
#include <QColorDialog>
#include <QPixmap>
#include <QShortcut>

#include <iostream>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    lastCalctime_normal(0),
    lastCalctime_specular(0),
    lastCalctime_displace(0),
    lastCalctime_ssao(0),
    stopQueue(false)
{
    ui->setupUi(this);

    supportedImageformats << "*.png" << "*.jpg" << "*.jpeg" << "*.tiff"
                          << "*.tif" << "*.ppm" << "*.bmp"  << "*.xpm"
                          << "*.tga" << "*.gif";

    //connect signals of GUI elements with slots of this class
    connectSignalSlots();

    //hide advanced settings and connect signals/slots to show them
    hideAdvancedSettings();

    //initialize graphicsview
    GraphicsScene *scene = new GraphicsScene();
    ui->graphicsView->setScene(scene);
    ui->graphicsView->setDragMode(QGraphicsView::ScrollHandDrag);
    scene->addText("Start by dragging images here.");
    ui->graphicsView->setRenderHints(QPainter::HighQualityAntialiasing
                                     | QPainter::SmoothPixmapTransform);
    ui->graphicsView->setAcceptDrops(true);

    //show default status message
    ui->statusBar->showMessage("Drag images into the empty preview window to load them.");

    //hide queue progressbar
    ui->progressBar_Queue->hide();
    
    // SSAO map generator is not ready yet, remove it from the UI
    ui->tabWidget->removeTab(4);

    //default UI colors
    useCustomUiColors = false;
    uiColorMainDefault = QColor("#444");
    uiColorTextDefault = QColor("#eee");
    uiColorGraphicsViewDefault = QColor(Qt::darkGray);
    uiColorMain = uiColorMainDefault;
    uiColorText = uiColorTextDefault;
    uiColorGraphicsView = uiColorGraphicsViewDefault;

    //read last window position and color settings from registry
    readSettings();

    //set UI colors
    setUiColors();

    //if the program was opened via "open with" by the OS,
    //extract the image paths from the arguments
    QStringList args = QCoreApplication::arguments();
    if(args.size() > 1) {
        QList<QUrl> imageUrls;

        for(int i = 1; i < args.size(); i++) {
            imageUrls.append(QUrl::fromLocalFile(args[i]));
        }

        loadMultipleDropped(imageUrls);
    }
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
        QFileInfo fileInfo(QFileInfo(urls.at(i).toLocalFile()));
        QString suffix = fileInfo.suffix().toLower();

        // The supportedImageformats list is of the form "*.jpg", "*.png", ...
        if(supportedImageformats.contains("*." + suffix, Qt::CaseInsensitive)) {
            //image format is supported, add to queue
            addImageToQueue(urls.at(i));
            //if it is the first valid image, load and preview it
            if(!loadedFirstValidImage) {
                load(urls.at(i));
                loadedFirstValidImage = true;
            }
        }
        else if(fileInfo.isDir()) {
            loadAllFromDir(urls.at(i));
        }
        else {
            containedInvalidFormat = true;
        }
    }

    if(containedInvalidFormat)
        QMessageBox::information(this, "Not All Images Loaded Into Queue",
                                 "Some images had unsupported formats and where not loaded into the queue!");
}

void MainWindow::loadAllFromDir(QUrl url) {
    // Load all images in the directory
    QDir directory(url.toLocalFile());

    QStringList fileList = directory.entryList(supportedImageformats, QDir::Files);
    QList<QUrl> urls;

    foreach(QString path, fileList) {
        QUrl subUrl = QUrl::fromLocalFile(directory.absolutePath() + QDir::separator() + path);
        urls.append(subUrl);
    }

    loadMultipleDropped(urls);
}

//load the image specified in the url
bool MainWindow::load(QUrl url) {
    if(!url.isValid()) {
        throw "[load] invalid url!";
        return false;
    }

    QFileInfo file(url.toLocalFile());

    if(file.isDir()) {
        loadAllFromDir(url);
        return false;
    }

    ui->statusBar->showMessage("loading Image: " + url.fileName());

    //load the image
    input = QImage(url.toLocalFile());
    ui->openGLWidget->addDiffuse(input);
    if(input.isNull()) {
        QString errorMessage("Image not loaded!");

        if(file.suffix().toLower() == "tga") {
            errorMessage.append("\nOnly uncompressed TGA files are supported.");
        }
        else {
            errorMessage.append("\nMost likely the image format is not supported.");
        }

        ui->statusBar->showMessage("Error: Image " + url.fileName() + " NOT loaded!", 5000);
        QMessageBox::information(this, "Error while loading image", errorMessage);
        return false;
    }

    //store the path the image was loaded from (for saving later)
    if(exportPath.isEmpty())
        setExportPath(url.adjusted(QUrl::RemoveFilename));
    loadedImagePath = url;

    //enable ui buttons
    ui->pushButton_calcNormal->setEnabled(true);
    ui->pushButton_calcSpec->setEnabled(true);
    ui->pushButton_calcDisplace->setEnabled(true);
    ui->pushButton_calcSsao->setEnabled(true);
    ui->spinBox_normalmapSize->setEnabled(true);
    enableAutoupdate(true);
    
    //extract R/G/B/A channels
    const int h = ui->label_channelRed->height();
    QImage inputSmall(input.scaled(h, h, Qt::KeepAspectRatio));
    IntensityMap red(inputSmall, IntensityMap::MAX, true, false, false, false);
    IntensityMap green(inputSmall, IntensityMap::MAX, false, true, false, false);
    IntensityMap blue(inputSmall, IntensityMap::MAX, false, false, true, false);
    IntensityMap alpha(inputSmall, IntensityMap::MAX, false, false, false, true);
    ui->label_channelRGBA->setPixmap(QPixmap::fromImage(inputSmall));
    ui->label_channelRed->setPixmap(QPixmap::fromImage(red.convertToQImage()));
    ui->label_channelGreen->setPixmap(QPixmap::fromImage(green.convertToQImage()));
    ui->label_channelBlue->setPixmap(QPixmap::fromImage(blue.convertToQImage()));
    ui->label_channelAlpha->setPixmap(QPixmap::fromImage(alpha.convertToQImage()));
    
    //algorithm to find best settings for KeepLargeDetail
    int imageSize = std::max(input.width(), input.height());
    
    int largeDetailScale = -0.037 * imageSize + 100;
    ui->checkBox_keepLargeDetail->setChecked(true);
    
    if(imageSize < 300) {
        ui->checkBox_keepLargeDetail->setChecked(false);
    }
    else if(imageSize > 2300) {
        largeDetailScale = 20;
    }
    
    ui->spinBox_largeDetailScale->setValue(largeDetailScale);
    
    //switch active tab to input
    ui->tabWidget->setCurrentIndex(0);

    //clear all previously generated images
    channelIntensity = QImage();
    normalmap = QImage();
    specmap = QImage();
    displacementmap = QImage();
    ssaomap = QImage();

    //display single image channels if the option was already chosen
    if(ui->radioButton_displayRGBA->isChecked())
        displayChannelIntensity();
    else
        preview(0);
    
    //image smaller than graphicsview: fitInView, then resetZoom (this way it is centered)
    //image larger than graphicsview: just fitInView
    fitInView();
    if(input.width() < ui->graphicsView->width() || input.height() < ui->graphicsView->height()) {
        resetZoom();
    }

    ui->statusBar->clearMessage();
    
    //enable button to save the maps
    ui->pushButton_save->setEnabled(true);

    return true;
}

//load images using the file dialog
void MainWindow::loadUserFilePath() {
    QString filter = "Image Formats (" + supportedImageformats.join(" ") + ")";

    QList<QUrl> urls = QFileDialog::getOpenFileUrls(this,
                                                    "Open Image File",
                                                    QDir::homePath(),
                                                    filter);
    loadMultipleDropped(urls);
}

void MainWindow::calcNormal() {
    if(input.isNull())
        return;

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
    
    //keep large detail settings
    bool keepLargeDetail = ui->checkBox_keepLargeDetail->isChecked();
    int largeDetailScale = ui->spinBox_largeDetailScale->value();
    double largeDetailHeight = ui->doubleSpinBox_largeDetailHeight->value();

    //scale input image if not 100%
    QImage inputScaled = input;
    int sizePercent = ui->spinBox_normalmapSize->value();
    if(sizePercent != 100) {
        int scaledWidth = calcPercentage(input.width(), sizePercent);
        int scaledHeight = calcPercentage(input.height(), sizePercent);

        inputScaled = input.scaled(scaledWidth, scaledHeight, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }
    
    //setup generator and calculate map
    NormalmapGenerator normalmapGenerator(mode, useRed, useGreen, useBlue, useAlpha);
    normalmap = normalmapGenerator.calculateNormalmap(inputScaled, kernel, strength, invert, tileable, keepLargeDetail, largeDetailScale, largeDetailHeight);
    normalmapRawIntensity = normalmapGenerator.getIntensityMap().convertToQImage();

    ui->openGLWidget->addNormal(normalmap);
}

void MainWindow::calcSpec() {
    if(input.isNull())
        return;

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
    double contrast = ui->doubleSpinBox_spec_contrast->value();

    //setup generator and calculate map
    SpecularmapGenerator specularmapGenerator(mode, redMultiplier, greenMultiplier, blueMultiplier, alphaMultiplier);
    specmap = specularmapGenerator.calculateSpecmap(input, scale, contrast);
    ui->openGLWidget->addSpecular(specmap);
}

//the displacement map is generated with the specularmapGenerator (similar controls and output needed)
void MainWindow::calcDisplace() {
    if(input.isNull())
        return;

    //color channel mode
    IntensityMap::Mode mode = IntensityMap::AVERAGE;
    if(ui->comboBox_mode_displace->currentIndex() == 0)
        mode = IntensityMap::AVERAGE;
    else if(ui->comboBox_mode_displace->currentIndex() == 1)
        mode = IntensityMap::MAX;

    //color channel multipliers to use
    double redMultiplier = ui->doubleSpinBox_displace_redMul->value();
    double greenMultiplier = ui->doubleSpinBox_displace_greenMul->value();
    double blueMultiplier = ui->doubleSpinBox_displace_blueMul->value();
    double alphaMultiplier = 0.0;
    double scale = ui->doubleSpinBox_displace_scale->value();
    double contrast = ui->doubleSpinBox_displace_contrast->value();

    //setup generator and calculate map
    SpecularmapGenerator specularmapGenerator(mode, redMultiplier, greenMultiplier, blueMultiplier, alphaMultiplier);
    displacementmap = specularmapGenerator.calculateSpecmap(input, scale, contrast);

    if(ui->checkBox_displace_blur->isChecked()) {
        int radius = ui->spinBox_displace_blurRadius->value();
        bool tileable = ui->checkBox_displace_blur_tileable->isChecked();

        IntensityMap inputMap(displacementmap, IntensityMap::AVERAGE);
        GaussianBlur filter;
        IntensityMap outputMap = filter.calculate(inputMap, radius, tileable);
        displacementmap = outputMap.convertToQImage();
    }

    ui->openGLWidget->addDisplacement(displacementmap);
}

void MainWindow::calcSsao() {
    if(input.isNull())
        return;

    //if no normalmap was created yet, calculate it
    if(normalmap.isNull()) {
        calcNormal();
    }

    //scale depthmap (can be smaller than normalmap because of KeepLargeDetail
    normalmapRawIntensity = normalmapRawIntensity.scaled(normalmap.width(), normalmap.height());
    float size = ui->doubleSpinBox_ssao_size->value();
    unsigned int samples = ui->spinBox_ssao_samples->value();
    unsigned int noiseTexSize = ui->spinBox_ssao_noiseTexSize->value();

    //setup generator and calculate map
    SsaoGenerator ssaoGenerator;
    ssaomap = ssaoGenerator.calculateSsaomap(normalmap, normalmapRawIntensity, size, samples, noiseTexSize);
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

    //preview in normalmap tab
    preview(1);
    
    //activate corresponding save checkbox
    ui->checkBox_queue_generateNormal->setChecked(true);
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

    //preview in specular map tab
    preview(2);
    
    //activate corresponding save checkbox
    ui->checkBox_queue_generateSpec->setChecked(true);
}

void MainWindow::calcDisplaceAndPreview() {
    ui->statusBar->showMessage("calculating displacementmap...");

    //timer for measuring calculation time
    QElapsedTimer timer;
    timer.start();

    //calculate map
    calcDisplace();

    //display time it took to calculate the map
    this->lastCalctime_displace = timer.elapsed();
    displayCalcTime(lastCalctime_displace, "displacementmap", 5000);

    //preview in displacement map tab
    preview(3);
    
    //activate corresponding save checkbox
    ui->checkBox_queue_generateDisplace->setChecked(true);
}

void MainWindow::calcSsaoAndPreview() {
    ui->statusBar->showMessage("calculating ambient occlusion map...");

    //timer for measuring calculation time
    QElapsedTimer timer;
    timer.start();

    //calculate map
    calcSsao();

    //display time it took to calculate the map
    this->lastCalctime_ssao = timer.elapsed();
    displayCalcTime(lastCalctime_ssao, "ambient occlusion map", 5000);

    //preview in ambient occlusion map tab
    preview(4);
    
    //activate corresponding save checkbox
    //ui->checkBox_queue_generate ssao ->setChecked(true); //missing until finished
}

void MainWindow::processQueue() {
    if(ui->listWidget_queue->count() == 0)
        return;

    if(!(ui->checkBox_queue_generateNormal->isChecked() ||
         ui->checkBox_queue_generateSpec->isChecked() ||
         ui->checkBox_queue_generateDisplace->isChecked())) {
        QMessageBox::information(this, "Nothing to do", "Select at least one map type to generate from the \"Save\" section");
        return;
    }

    if(!exportPath.isValid()) {
        QMessageBox::information(this, "Invalid Export Path", "Export path is invalid!");
        return;
    }

    //enable stop button
    ui->pushButton_stopProcessingQueue->setEnabled(true);
    //show progress bar and adjust maximum to queue size
    ui->progressBar_Queue->show();
    ui->progressBar_Queue->setMaximum(ui->listWidget_queue->count());

    for(int i = 0; i < ui->listWidget_queue->count() && !stopQueue; i++)
    {
        QueueItem *item = (QueueItem*)(ui->listWidget_queue->item(i));

        //display status
        ui->statusBar->showMessage("Processing Image \"" + item->text() + "\"");
        ui->progressBar_Queue->setValue(i + 1);
        ui->listWidget_queue->item(i)->setSelected(true);

        //load image
        load(item->getUrl());
        
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
    //hide queue progress bar
    ui->progressBar_Queue->hide();

    //enable "Open Export Folder" gui button
    ui->pushButton_openExportFolder->setEnabled(true);
}

//tell the queue to stop processing
void MainWindow::stopProcessingQueue() {
    stopQueue = true;
}

//save maps using the file dialog
void MainWindow::saveUserFilePath() {
    if(input.isNull())
        return;

    QFileDialog::Options options(QFileDialog::DontConfirmOverwrite);
    QUrl url = QFileDialog::getSaveFileUrl(this, "Save as", loadedImagePath,
                                           "Image Formats (*.png *.jpg *.jpeg *.tiff *.ppm *.bmp *.xpm)",
                                           0, options);

    if(!url.isValid() || url.toLocalFile().isEmpty())
        return;

    save(url);
}

void MainWindow::save(QUrl url) {
    //if saving process was aborted or input image is empty
    if(!url.isValid() || input.isNull())
        return;

    QString path = url.toLocalFile();

    //if no file suffix was chosen, automatically use the PNG format
    QFileInfo file(path);
    if(!file.baseName().isEmpty() && file.suffix().isEmpty())
        path += ".png";

    QString suffix = file.suffix();
    //Qt can only read tga, saving is not supported
    if(suffix.toLower() == "tga")
        suffix = "png";

    //append a suffix to the map names (result: path/original_normal.png)
    QString name_normal = file.absolutePath() + "/" + file.baseName() + "_normal." + suffix;
    QString name_specular = file.absolutePath() + "/" + file.baseName() + "_spec." + suffix;
    QString name_displace = file.absolutePath() + "/" + file.baseName() + "_displace." + suffix;

    bool successfullySaved = true;
    
    if(ui->checkBox_queue_generateNormal->isChecked()) {
        if(normalmap.isNull())
            ui->statusBar->showMessage("calculating normalmap...");
            calcNormal();
        
        successfullySaved &= normalmap.save(name_normal);
    }    
    
    if(ui->checkBox_queue_generateSpec->isChecked()) {
        if(specmap.isNull())
            ui->statusBar->showMessage("calculating specularmap...");
            calcSpec();
        
        successfullySaved &= specmap.save(name_specular);
    }

    if(ui->checkBox_queue_generateDisplace->isChecked()) {
        if(displacementmap.isNull())
            ui->statusBar->showMessage("calculating displacementmap...");
            calcDisplace();
        
        successfullySaved &= displacementmap.save(name_displace);
    }
    
    if(successfullySaved)
        ui->statusBar->showMessage("Maps successfully saved", 4000);
    else
        QMessageBox::information(this, "Maps not saved", "One or more of the maps was NOT saved!");
    
    //store export path
    setExportPath(url.adjusted(QUrl::RemoveFilename | QUrl::StripTrailingSlash));
    //enable "Open Export Folder" gui button
    ui->pushButton_openExportFolder->setEnabled(true);
}

bool MainWindow::setExportPath(QUrl path) {
    QDir testDir(path.toLocalFile());
    if(testDir.exists() && !path.toLocalFile().isEmpty()) {
        std::cout << "set export path to " + path.toLocalFile().toStdString() << std::endl;
        this->exportPath = path;
        //stop highlighting
        ui->lineEdit_outputPath->setStyleSheet("");
        ui->lineEdit_outputPath->setText(path.toLocalFile());
        return true;
    }
    else if(path.toLocalFile().isEmpty()) {
        //keep previous highlighting
        return false;
    }
    else {
        //hightlight wrong path in red
        ui->lineEdit_outputPath->setStyleSheet("color: red;");
        return false;
    }
}

//change the path the queue exports the maps to
void MainWindow::changeOutputPathQueueDialog() {
    QUrl startUrl = QDir::homePath();
    if(exportPath.isValid())
        startUrl = exportPath;

    QUrl path = QFileDialog::getExistingDirectoryUrl(this,
                                                     "Choose Export Folder",
                                                     startUrl);
    setExportPath(path);
}

void MainWindow::editOutputPathQueue() {
    QString rawPath = ui->lineEdit_outputPath->text();
    setExportPath(QUrl::fromLocalFile(rawPath));
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
    {
        //input
        if(!input.isNull()) {
            if(ui->radioButton_displayRGBA->isChecked()) {
                QGraphicsPixmapItem *pixmap = ui->graphicsView->scene()->addPixmap(QPixmap::fromImage(input));
                pixmap->setTransformationMode(Qt::SmoothTransformation);
            }
            else {
                QGraphicsPixmapItem *pixmap = ui->graphicsView->scene()->addPixmap(QPixmap::fromImage(channelIntensity));
                pixmap->setTransformationMode(Qt::SmoothTransformation);
            }
        }
        break;
    }
    case 1:
    {
        //normal
        if(!input.isNull() && normalmap.isNull()) {
            //if an image was loaded and a normalmap was not yet generated and the image is not too large
            //automatically generate the normalmap
            calcNormalAndPreview();
        }
        QGraphicsPixmapItem *pixmap = ui->graphicsView->scene()->addPixmap(QPixmap::fromImage(normalmap));
        pixmap->setTransformationMode(Qt::SmoothTransformation);
        //display size of the image
        normalmapSizeChanged();
        break;
    }
    case 2:
    {
        //spec
        if(!input.isNull() && specmap.isNull()) {
            //if an image was loaded and a specmap was not yet generated and the image is not too large
            //automatically generate the specmap
            calcSpecAndPreview();
        }
        QGraphicsPixmapItem *pixmap = ui->graphicsView->scene()->addPixmap(QPixmap::fromImage(specmap));
        pixmap->setTransformationMode(Qt::SmoothTransformation);
        break;
    }
    case 3:
    {
        //displacement
        if(!input.isNull() && displacementmap.isNull()) {
            //if an image was loaded and a dispmap was not yet generated and the image is not too large
            //automatically generate the displacementmap
            calcDisplaceAndPreview();
        }
        QGraphicsPixmapItem *pixmap = ui->graphicsView->scene()->addPixmap(QPixmap::fromImage(displacementmap));
        pixmap->setTransformationMode(Qt::SmoothTransformation);
        break;
    }
    case 4:
    {
        //ambient occlusion
        if(!input.isNull() && ssaomap.isNull()) {
            //if an image was loaded and a ssaomap was not yet generated and the image is not too large
            //automatically generate the ambient occlusion map
            calcSsaoAndPreview();
        }
        QGraphicsPixmapItem *pixmap = ui->graphicsView->scene()->addPixmap(QPixmap::fromImage(ssaomap));
        pixmap->setTransformationMode(Qt::SmoothTransformation);
        break;
    }
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
    if(input.isNull())
        return;

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
    case 4:
        if(lastCalctime_ssao < autoUpdateThreshold_ms)
            calcSsaoAndPreview();
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
    std::cout << mapType.toStdString() << " for image " << loadedImagePath.fileName().toStdString()
              << " calculated (" << calcTime_ms << "ms)" << std::endl;
    ui->statusBar->clearMessage();
    QString msg = generateElapsedTimeMsg(calcTime_ms, mapType);
    ui->statusBar->showMessage(msg, duration_ms);
    ui->label_autoUpdate_lastCalcTime->setText("(Last Calc. Time: " + QString::number((double)calcTime_ms / 1000.0) + "s)");
    
    if(calcTime_ms < ui->doubleSpinBox_autoUpdateThreshold->value() * 1000) {
        //calcTime was below the threshold, set textcolor to green
        ui->label_autoUpdate_lastCalcTime->setStyleSheet("QLabel {color: #00AA00;}");
    }
    else {
        //calcTime was above threshold, set textcolor to red to signal user the time was too long for autoupdate
        ui->label_autoUpdate_lastCalcTime->setStyleSheet("QLabel {color: red;}");
    }
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

    //QIcon icon(QPixmap(url.toLocalFile()).scaled(64, 64, Qt::KeepAspectRatio));
    //item->setIcon(icon);

    //ui->listWidget_queue->addItem(item);
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

//calculates the size preview text (e.g. "1024 x 1024 px")
void MainWindow::normalmapSizeChanged() {
    int sizePercent = ui->spinBox_normalmapSize->value();
    QString text = QString::number(calcPercentage(input.width(), sizePercent));
    text.append(" x ");
    text.append(QString::number(calcPercentage(input.height(), sizePercent)));
    text.append(" px");
    ui->label_normalmapSize->setText(text);
}

// Used to scale down the input image if a value less than 100 
// is set in spinBox_normalmapSize. We have to make sure size is at least 1
int MainWindow::calcPercentage(int value, int percentage) {
    const int newValue = (((double)value / 100.0) * percentage);
    return std::max(newValue, 1);
}

void MainWindow::showAboutDialog() {
    AboutDialog *dialog = new AboutDialog(this, this);
    dialog->show();
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
    connect(ui->pushButton_calcSsao, SIGNAL(clicked()), this, SLOT(calcSsaoAndPreview()));
    //switch between tabs
    connect(ui->tabWidget, SIGNAL(tabBarClicked(int)), this, SLOT(preview(int)));
    connect(ui->tabWidget, SIGNAL(currentChanged(int)), this, SLOT(preview(int)));
    //display channel intensity
    //radio buttons
    connect(ui->radioButton_displayRGBA, SIGNAL(clicked()), this, SLOT(displayChannelIntensity()));
    connect(ui->radioButton_displayRed, SIGNAL(clicked()), this, SLOT(displayChannelIntensity()));
    connect(ui->radioButton_displayGreen, SIGNAL(clicked()), this, SLOT(displayChannelIntensity()));
    connect(ui->radioButton_displayBlue, SIGNAL(clicked()), this, SLOT(displayChannelIntensity()));
    connect(ui->radioButton_displayAlpha, SIGNAL(clicked()), this, SLOT(displayChannelIntensity()));
    //labels (channel images)
    connect(ui->label_channelRGBA, SIGNAL(clicked()), ui->radioButton_displayRGBA, SLOT(click()));
    connect(ui->label_channelRed, SIGNAL(clicked()), ui->radioButton_displayRed, SLOT(click()));
    connect(ui->label_channelGreen, SIGNAL(clicked()), ui->radioButton_displayGreen, SLOT(click()));
    connect(ui->label_channelBlue, SIGNAL(clicked()), ui->radioButton_displayBlue, SLOT(click()));
    connect(ui->label_channelAlpha, SIGNAL(clicked()), ui->radioButton_displayAlpha, SLOT(click()));
    //autoupdate after changed values
    // spec autoupdate
    connect(ui->doubleSpinBox_spec_redMul, SIGNAL(valueChanged(double)), this, SLOT(autoUpdate()));
    connect(ui->doubleSpinBox_spec_greenMul, SIGNAL(valueChanged(double)), this, SLOT(autoUpdate()));
    connect(ui->doubleSpinBox_spec_blueMul, SIGNAL(valueChanged(double)), this, SLOT(autoUpdate()));
    connect(ui->doubleSpinBox_spec_alphaMul, SIGNAL(valueChanged(double)), this, SLOT(autoUpdate()));
    connect(ui->doubleSpinBox_spec_scale, SIGNAL(valueChanged(double)), this, SLOT(autoUpdate()));
    connect(ui->comboBox_mode_spec, SIGNAL(currentIndexChanged(int)), this, SLOT(autoUpdate()));
    connect(ui->doubleSpinBox_spec_contrast, SIGNAL(valueChanged(double)), this, SLOT(autoUpdate()));
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
    connect(ui->spinBox_normalmapSize, SIGNAL(valueChanged(int)), this, SLOT(autoUpdate()));
    connect(ui->checkBox_keepLargeDetail, SIGNAL(clicked()), this, SLOT(autoUpdate()));
    connect(ui->spinBox_largeDetailScale, SIGNAL(valueChanged(int)), this, SLOT(autoUpdate()));
    connect(ui->doubleSpinBox_largeDetailHeight, SIGNAL(valueChanged(double)), this, SLOT(autoUpdate()));
    // displacement autoupdate
    connect(ui->doubleSpinBox_displace_redMul, SIGNAL(valueChanged(double)), this, SLOT(autoUpdate()));
    connect(ui->doubleSpinBox_displace_greenMul, SIGNAL(valueChanged(double)), this, SLOT(autoUpdate()));
    connect(ui->doubleSpinBox_displace_blueMul, SIGNAL(valueChanged(double)), this, SLOT(autoUpdate()));
    connect(ui->doubleSpinBox_displace_scale, SIGNAL(valueChanged(double)), this, SLOT(autoUpdate()));
    connect(ui->comboBox_mode_displace, SIGNAL(currentIndexChanged(int)), this, SLOT(autoUpdate()));
    connect(ui->doubleSpinBox_displace_contrast, SIGNAL(valueChanged(double)), this, SLOT(autoUpdate()));
    connect(ui->checkBox_displace_blur, SIGNAL(stateChanged(int)), this, SLOT(autoUpdate()));
    connect(ui->checkBox_displace_blur_tileable, SIGNAL(stateChanged(int)), this, SLOT(autoUpdate()));
    connect(ui->spinBox_displace_blurRadius, SIGNAL(valueChanged(int)), this, SLOT(autoUpdate()));
    // ssao autoupdate
    connect(ui->doubleSpinBox_ssao_size, SIGNAL(valueChanged(double)), this, SLOT(autoUpdate()));
    //graphicsview drag and drop
    connect(ui->graphicsView, SIGNAL(singleImageDropped(QUrl)), this, SLOT(loadSingleDropped(QUrl)));
    connect(ui->graphicsView, SIGNAL(multipleImagesDropped(QList<QUrl>)), this, SLOT(loadMultipleDropped(QList<QUrl>)));
    //graphicsview rightclick/middleclick/zoom
    connect(ui->graphicsView, SIGNAL(rightClick()), this, SLOT(resetZoom()));
    connect(ui->graphicsView, SIGNAL(middleClick()), this, SLOT(fitInView()));
    connect(ui->graphicsView, SIGNAL(zoomIn()), this, SLOT(zoomIn()));
    connect(ui->graphicsView, SIGNAL(zoomOut()), this, SLOT(zoomOut()));
    //queue (item widget)
    connect(ui->pushButton_removeImagesFromQueue, SIGNAL(clicked()), this, SLOT(removeImagesFromQueue()));
    connect(ui->pushButton_processQueue, SIGNAL(clicked()), this, SLOT(processQueue()));
    connect(ui->pushButton_stopProcessingQueue, SIGNAL(clicked()), this, SLOT(stopProcessingQueue()));
    connect(ui->pushButton_changeOutputPath_Queue, SIGNAL(clicked()), this, SLOT(changeOutputPathQueueDialog()));
    connect(ui->lineEdit_outputPath, SIGNAL(editingFinished()), this, SLOT(editOutputPathQueue()));
    connect(ui->listWidget_queue, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(queueItemDoubleClicked(QListWidgetItem*)));
    //queue drag and drop
    connect(ui->listWidget_queue, SIGNAL(singleImageDropped(QUrl)), this, SLOT(loadSingleDropped(QUrl)));
    connect(ui->listWidget_queue, SIGNAL(multipleImagesDropped(QList<QUrl>)), this, SLOT(loadMultipleDropped(QList<QUrl>)));
    //normalmap size preview text
    connect(ui->spinBox_normalmapSize, SIGNAL(valueChanged(int)), this, SLOT(normalmapSizeChanged()));
    //"About" button
    connect(ui->pushButton_about, SIGNAL(clicked()), this, SLOT(showAboutDialog()));

    //Shortcuts
    QShortcut *save = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_S), this, 0, 0, Qt::ApplicationShortcut);
    connect(save, SIGNAL(activated()), this, SLOT(saveUserFilePath()));
    QShortcut *load = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_O), this, 0, 0, Qt::ApplicationShortcut);
    connect(load, SIGNAL(activated()), this, SLOT(loadUserFilePath()));

    QShortcut *tab_1 = new QShortcut(QKeySequence(Qt::Key_1), this, 0, 0, Qt::ApplicationShortcut);
    connect(tab_1, SIGNAL(activated()), this, SLOT(switchToTab1()));
    QShortcut *tab_2 = new QShortcut(QKeySequence(Qt::Key_2), this, 0, 0, Qt::ApplicationShortcut);
    connect(tab_2, SIGNAL(activated()), this, SLOT(switchToTab2()));
    QShortcut *tab_3 = new QShortcut(QKeySequence(Qt::Key_3), this, 0, 0, Qt::ApplicationShortcut);
    connect(tab_3, SIGNAL(activated()), this, SLOT(switchToTab3()));
    QShortcut *tab_4 = new QShortcut(QKeySequence(Qt::Key_4), this, 0, 0, Qt::ApplicationShortcut);
    connect(tab_4, SIGNAL(activated()), this, SLOT(switchToTab4()));

}

void MainWindow::switchToTab1() {
    ui->tabWidget->setCurrentIndex(0);
}

void MainWindow::switchToTab2() {
    ui->tabWidget->setCurrentIndex(1);
}

void MainWindow::switchToTab3() {
    ui->tabWidget->setCurrentIndex(2);
}

void MainWindow::switchToTab4() {
    ui->tabWidget->setCurrentIndex(3);
}

void MainWindow::hideAdvancedSettings() {
    //Normalmap
    //"Alpha" checkbox
    ui->checkBox_useAlpha_normal->setVisible(false);
    connect(ui->checkBox_advanced_normal, SIGNAL(clicked(bool)), ui->checkBox_useAlpha_normal, SLOT(setVisible(bool)));
    //"Average/Max" combobox
    ui->comboBox_mode_normal->setVisible(false);
    connect(ui->checkBox_advanced_normal, SIGNAL(clicked(bool)), ui->comboBox_mode_normal, SLOT(setVisible(bool)));
    //"Method" label and combobox
    ui->comboBox_method->setVisible(false);
    connect(ui->checkBox_advanced_normal, SIGNAL(clicked(bool)), ui->comboBox_method, SLOT(setVisible(bool)));
    ui->label_method_normal->setVisible(false);
    connect(ui->checkBox_advanced_normal, SIGNAL(clicked(bool)), ui->label_method_normal, SLOT(setVisible(bool)));
}

void MainWindow::closeEvent(QCloseEvent* event) {
    writeSettings();
}

//write window size, position etc. to registry
void MainWindow::writeSettings()
{
    QSettings qsettings( "simon", "normalmapgenerator" );

    qsettings.beginGroup( "mainwindow" );

    qsettings.setValue( "geometry", saveGeometry() );
    qsettings.setValue( "savestate", saveState() );
    qsettings.setValue( "maximized", isMaximized() );

    if ( !isMaximized() ) {
        qsettings.setValue( "pos", pos() );
        qsettings.setValue( "size", size() );
    }

    // Custom UI colors
    qsettings.setValue("use_custom_ui_colors", useCustomUiColors);
    qsettings.setValue("ui_color_main", uiColorMain.rgba());
    qsettings.setValue("ui_color_text", uiColorText.rgba());
    qsettings.setValue("ui_color_graphicsview", uiColorGraphicsView.rgba());

    qsettings.endGroup();
}

//read window size, position etc. from registry
void MainWindow::readSettings()
{
    QSettings qsettings( "simon", "normalmapgenerator" );

    qsettings.beginGroup( "mainwindow" );

    restoreGeometry(qsettings.value( "geometry", saveGeometry() ).toByteArray());
    restoreState(qsettings.value( "savestate", saveState() ).toByteArray());
    move(qsettings.value( "pos", pos() ).toPoint());
    resize(qsettings.value( "size", size() ).toSize());

    if ( qsettings.value( "maximized", isMaximized() ).toBool() )
        showMaximized();

    // Custom UI colors
    if(qsettings.contains("use_custom_ui_colors"))
        useCustomUiColors = qsettings.value("use_custom_ui_colors").toBool();
    if(qsettings.contains("ui_color_main"))
        uiColorMain.setRgba(qsettings.value("ui_color_main").toUInt());
    if(qsettings.contains("ui_color_text"))
        uiColorText.setRgba(qsettings.value("ui_color_text").toUInt());
    if(qsettings.contains("ui_color_graphicsview"))
        uiColorGraphicsView.setRgba(qsettings.value("ui_color_graphicsview").toUInt());

    qsettings.endGroup();
}

void MainWindow::setUiColors() {
    if(useCustomUiColors) {
        QFile file(":/stylesheets/resources/stylesheets/theme_dark.qss");
        if(file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QString stylesheet(file.readAll());
            file.close();

            // Replace colorcodes with colors (">uiColorMain" -> "#444")
            stylesheet.replace("<uiColorMain>", uiColorMain.name());
            stylesheet.replace("<uiColorText>", uiColorText.name());
            stylesheet.replace("<uiColorGraphicsView>", uiColorGraphicsView.name());

            // Compute lighter/darker color variants
            int add = 9;
            QColor uiColorMainLighter_0(std::min(uiColorMain.red() + add, 255),
                                        std::min(uiColorMain.green() + add, 255),
                                        std::min(uiColorMain.blue() + add, 255));

            add = 35;
            QColor uiColorMainLighter_1(std::min(uiColorMain.red() + add, 255),
                                        std::min(uiColorMain.green() + add, 255),
                                        std::min(uiColorMain.blue() + add, 255));

            add = 50;
            QColor uiColorMainLighter_2(std::min(uiColorMain.red() + add, 255),
                                        std::min(uiColorMain.green() + add, 255),
                                        std::min(uiColorMain.blue() + add, 255));

            add = -18;
            QColor uiColorMainDarker(std::max(uiColorMain.red() + add, 0),
                                     std::max(uiColorMain.green() + add, 0),
                                     std::max(uiColorMain.blue() + add, 0));

            stylesheet.replace("<uiColorMainLighter_0>", uiColorMainLighter_0.name());
            stylesheet.replace("<uiColorMainLighter_1>", uiColorMainLighter_1.name());
            stylesheet.replace("<uiColorMainLighter_2>", uiColorMainLighter_2.name());
            stylesheet.replace("<uiColorMainDarker>", uiColorMainLighter_0.name());

            this->setStyleSheet(stylesheet);
        }
    }
    else {
        // Use system colors for everything except the graphicsview background
        setStyleSheet("QGraphicsView { background-color: #888}");
    }
}

bool MainWindow::getUseCustomUiColors() {
    return useCustomUiColors;
}

void MainWindow::setUseCustomUiColors(bool value) {
    useCustomUiColors = value;
    setUiColors();
}

QColor MainWindow::getUiColorMain() {
    return uiColorMain;
}

void MainWindow::setUiColorMain(QColor value) {
    uiColorMain = value;
    setUiColors();
}

QColor MainWindow::getUiColorText() {
    return uiColorText;
}

void MainWindow::setUiColorText(QColor value) {
    uiColorText = value;
    setUiColors();
}

QColor MainWindow::getUiColorGraphicsView() {
    return uiColorGraphicsView;
}

void MainWindow::setUiColorGraphicsView(QColor value) {
    uiColorGraphicsView = value;
    setUiColors();
}

void MainWindow::resetUiColors() {
    uiColorMain = uiColorMainDefault;
    uiColorText = uiColorTextDefault;
    uiColorGraphicsView = uiColorGraphicsViewDefault;
    setUiColors();
}

void MainWindow::on_horizontalSlider_valueChanged(int value)
{
    ui->openGLWidget->setDepthValue(float(value) / 10.0f);
}

void MainWindow::on_horizontalSlider_2_valueChanged(int value)
{
    ui->openGLWidget->setPartitionFrequency(value);
}

void MainWindow::on_checkBox_clicked(bool checked)
{
    ui->openGLWidget->setRotating(checked);
}
