#include "window.h"
#include <QDebug>
#include <QMouseEvent>

Window::Window(QGroupBox *)
{
}

Window::~Window()
{
    teardownGL();
}

void Window::update()
{
    if(areWeRotating)
        scene.setTransformRotation(0.1f, 0.1f);

    program->bind();
    program->setUniformValue(u_modelToWorld, scene.getModelToWorldMatrix());
    program->release();
    QWidget::update();
}

void Window::initializeGL()
{
    initializeOpenGLFunctions();
    scene.initializeSceneOpenGLFunctions();
    scene.calculateVertices();
    connect(this, SIGNAL(frameSwapped()), this, SLOT(update()));

    glClearColor(0.5f, 0.5f, 0.5f, 1.0f);

    // Add shaders source code
    program = new QOpenGLShaderProgram();
    bool success = true;
    success &= program->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/shader.vert");
    success &= program->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/shader.frag");
    success &= program->addShaderFromSourceFile(QOpenGLShader::TessellationControl, ":/shaders/shader.tessc");
    success &= program->addShaderFromSourceFile(QOpenGLShader::TessellationEvaluation, ":/shaders/shader.tesse");
    success &= program->addShaderFromSourceFile(QOpenGLShader::Geometry, ":/shaders/shader.geom");
    bool showErrorMessage = !success;
    
    // Emit a signal so the UI can react to shader compilation errors by displaying a message
    if(success) {
        program->link();
        program->bind();
        emit compiledShaders(showErrorMessage);
    } else {
        qDebug() << "Shader compilation failed";
        emit compiledShaders(showErrorMessage);
        return;
    }

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    glDepthFunc(GL_LESS);


    // uniform matrices locations
    u_modelToWorld = program->uniformLocation("modelToWorld");
    u_cameraToView = program->uniformLocation("cameraToView");
    u_worldToCamera = program->uniformLocation("worldToCamera");

    u_cameraPosition = program->uniformLocation("cameraPos");

    // light stuff
    u_lightColor = program->uniformLocation("light.Color");
    u_lightAmbI = program->uniformLocation("light.AmbientIntensity");
    u_lightDifI = program->uniformLocation("light.DiffuseIntensity");
    u_lightDir = program->uniformLocation("light.Direction");
    u_lightSpec = program->uniformLocation("light.SpecPower");
    u_lightMatShiness = program->uniformLocation("light.MatShines");

    // renderer properties
    u_depth = program->uniformLocation("depth");
    u_partFreq = program->uniformLocation("partFreq");

    u_applyingDiffuse         = program->uniformLocation("applyingDiffuse");
    u_applyingNormal          = program->uniformLocation("applyingNormal");
    u_applyingSpecular        = program->uniformLocation("applyingSpecular");
    u_applyingDisplacement    = program->uniformLocation("applyingDisplacement");

    // Create Buffer for vao
    bufferForVertices.create();
    bufferForVertices.bind();
    bufferForVertices.setUsagePattern(QOpenGLBuffer::StaticDraw);
    bufferForVertices.allocate(scene.vertices, sizeof(Vertex) *
                               sizeof(scene.vertices) / sizeof(scene.vertices[0]));

    // Create Vertex Array Object
    vao.create();
    vao.bind();


    program->setPatchVertexCount(4);


    program->enableAttributeArray(0);
    program->setAttributeBuffer(0, GL_FLOAT, 0, 3, sizeof(Vertex));

    // THIS GOTTA BE IN THE PLACE THERE LIGHT STATS ARE CHANGING
    ///////////////////////////////////////////////////////////////
    program->setUniformValue(u_lightColor, light.Color());
    program->setUniformValue(u_lightAmbI, light.AmbientIntensity());
    program->setUniformValue(u_lightDifI, light.DiffuseIntensity());
    program->setUniformValue(u_lightSpec, light.SpecularPower());
    program->setUniformValue(u_lightDir, light.Direction());
    program->setUniformValue(u_lightMatShiness, light.MatertialShines());
    ///////////////////////////////////////////////////////////////


    program->setUniformValue(u_cameraToView, scene.getCameraToViewMatrix());
    program->setUniformValue(u_worldToCamera, scene.getWorldToCameraMatrix());
    program->setUniformValue(u_modelToWorld, scene.getModelToWorldMatrix());

    program->setUniformValue(u_cameraPosition, scene.getCameraPosition());

    program->setUniformValue(u_depth, depth);
    program->setUniformValue(u_partFreq, partitionFrequency);

    vao.release();
    bufferForVertices.release();
    program->release();
}

void Window::keyPressEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_W)
        scene.translateCameraBy(cameraSpeed, scene.forwardCamera());
    if(event->key() == Qt::Key_S)
        scene.translateCameraBy(-cameraSpeed, scene.forwardCamera());
    if(event->key() == Qt::Key_A)
        scene.translateCameraBy(-cameraSpeed, scene.rightCamera());
    if(event->key() == Qt::Key_D)
        scene.translateCameraBy(cameraSpeed, scene.rightCamera());
    if(event->key() == Qt::Key_Q)
        scene.translateCameraBy(-cameraSpeed, scene.upCamera());
    if(event->key() == Qt::Key_E)
        scene.translateCameraBy(cameraSpeed, scene.upCamera());
    if(event->key() == Qt::Key_I)
        light.rotateUp();
    if(event->key() == Qt::Key_K)
        light.rotateDown();
    if(event->key() == Qt::Key_J)
        light.rotateLeft();
    if(event->key() == Qt::Key_L)
        light.rotateRight();

    program->bind();
    program->setUniformValue(u_worldToCamera, scene.getWorldToCameraMatrix());
    program->setUniformValue(u_cameraPosition, scene.getCameraPosition());
    program->setUniformValue(u_lightDir, light.Direction());
    program->setUniformValue(u_lightMatShiness, light.MatertialShines());
    program->release();

    QWidget::update();
}

void Window::mousePressEvent(QMouseEvent *pe)
{
    this->setFocus();
    pressed = true;
    ptrMousePosition = pe->pos();
}

void Window::mouseMoveEvent(QMouseEvent *pe)
{
    if(pressed)
    {
        scene.setCameraRotation(
                    rotatingSpeed * (GLfloat)(pe->x() - ptrMousePosition.x()) / width(),
                    scene.upLocalCamera());
        scene.setCameraRotation(
                    rotatingSpeed * (GLfloat)(pe->y() - ptrMousePosition.y()) / height(),
                    scene.rightCamera());
        ptrMousePosition = pe->pos();

        // new camera position and world to camera matrice going to the shader
        program->bind();
        program->setUniformValue(u_worldToCamera, scene.getWorldToCameraMatrix());
        program->setUniformValue(u_cameraPosition, scene.getCameraPosition());
        program->release();

        QWidget::update();
    }
}

void Window::resizeGL(int width, int height)
{
    scene.changeProjection(width, height);
    program->bind();
    program->setUniformValue(u_cameraToView, scene.getCameraToViewMatrix());
    program->release();
    QWidget::update();
}

void Window::mouseReleaseEvent(QMouseEvent *)
{
    pressed = false;
    QWidget::update();
}

void Window::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT);
    glClear(GL_DEPTH_BUFFER_BIT);
    program->bind();

    if(scene.checkForReady())
    {        
        if(scene.applyingDiffuse)
        // diffuse map attaching
        {
            glActiveTexture(GL_TEXTURE0);
            this->diffuseMap->bind();
            glUniform1i(program->uniformLocation("diffuseMap"), 0);
        }
        if(scene.applyingDisplacement)
        // displacement map attaching
        {
            glActiveTexture(GL_TEXTURE1);
            this->displacementMap->bind();
            glUniform1i(program->uniformLocation("displacementMap"), 1);
        }
        if(scene.applyingNormal)
        // normal map attaching
        {
            glActiveTexture(GL_TEXTURE2);
            this->normalMap->bind();
            glUniform1i(program->uniformLocation("normalMap"), 2);
        }
        if(scene.applyingSpecular)
        // specular map attaching
        {
            glActiveTexture(GL_TEXTURE3);
            this->specularMap->bind();
            glUniform1i(program->uniformLocation("specularMap"), 3);
        }
        vao.bind();

        glDrawArrays(GL_PATCHES, 0, sizeof(scene.vertices) / sizeof(scene.vertices[0]));

        if(scene.applyingDiffuse)
            this->diffuseMap->release();
        if(scene.applyingDisplacement)
            this->displacementMap->release();
        if(scene.applyingNormal)
            this->normalMap->release();
        if(scene.applyingSpecular)
            this->specularMap->release();
        vao.release();
    }
    program->release();
}

void Window::teardownGL()
{
    vao.destroy();
    bufferForVertices.destroy();
    delete program;
}

void Window::addDisplacement(QImage &displacementMap)
{
    scene.addDisplacementMap(displacementMap, *program);
    this->displacementMap = new QOpenGLTexture(scene.getDisplacement().mirrored());
    this->displacementMap->setMinificationFilter(QOpenGLTexture::Linear);
    this->displacementMap->setMagnificationFilter(QOpenGLTexture::Linear);

    setApplyingDisplacement(true);
}

void Window::addDiffuse(QImage &diffuseMap)
{   
    scene.addDiffuseMap(diffuseMap, *program);
    this->diffuseMap = new QOpenGLTexture(scene.getDiffuse().mirrored());
    this->diffuseMap->setMinificationFilter(QOpenGLTexture::Linear);
    this->diffuseMap->setMagnificationFilter(QOpenGLTexture::Linear);

    setApplyingDiffuse(true);
}

void Window::addNormal(QImage &normalMap)
{
    scene.addNormalMap(normalMap, *program);
    this->normalMap = new QOpenGLTexture(scene.getNormal().mirrored());
    this->normalMap->setMinificationFilter(QOpenGLTexture::Linear);
    this->normalMap->setMagnificationFilter(QOpenGLTexture::Linear);

    setApplyingNormal(true);
}

void Window::addSpecular(QImage &specularMap)
{
    scene.addSpecularMap(specularMap, *program);
    this->specularMap = new QOpenGLTexture(scene.getSpecular().mirrored());
    this->specularMap->setMinificationFilter(QOpenGLTexture::Linear);
    this->specularMap->setMagnificationFilter(QOpenGLTexture::Linear);

    setApplyingSpecular(true);
}

void Window::setDepthValue(float newDepth)
{
    depth = newDepth;

    program->bind();
    program->setUniformValue(u_depth, depth);
    program->release();
}

void Window::setPartitionFrequency(int newPartFreq)
{
    partitionFrequency = newPartFreq;

    program->bind();
    program->setUniformValue(u_partFreq, partitionFrequency);
    program->release();
}

void Window::setRotating(bool value)
{
    areWeRotating = value;
}

void Window::setRoughness(float value)
{
    light.setMaterialShines(value);

    program->bind();
    program->setUniformValue(u_lightMatShiness, light.MatertialShines());
    program->release();
}

void Window::setApplyingDiffuse(bool value)
{
    scene.applyingDiffuse = value;
    program->bind();
    program->setUniformValue(u_applyingDiffuse, scene.applyingDiffuse);
    program->release();
}

void Window::setApplyingNormal(bool value)
{
    scene.applyingNormal = value;
    program->bind();
    program->setUniformValue(u_applyingNormal, scene.applyingNormal);
    program->release();
}

void Window::setApplyingSpecular(bool value)
{
    scene.applyingSpecular = value;
    program->bind();
    program->setUniformValue(u_applyingSpecular, scene.applyingSpecular);
    program->release();
}

void Window::setApplyingDisplacement(bool value)
{
    scene.applyingDisplacement = value;
    program->bind();
    program->setUniformValue(u_applyingDisplacement, scene.applyingDisplacement);
    program->release();
}
