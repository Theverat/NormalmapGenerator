#include "window.h"
#include <QDebug>
#include <QString>
#include <QMouseEvent>
#include "src_gui/mainwindow.h"

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

    // Add shaders sourse code
    program = new QOpenGLShaderProgram();
    program->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/shader.vert");
    program->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/shader.frag");
    program->addShaderFromSourceFile(QOpenGLShader::TessellationControl, ":/shaders/shader.tessc");
    program->addShaderFromSourceFile(QOpenGLShader::TessellationEvaluation, ":/shaders/shader.tesse");
    program->addShaderFromSourceFile(QOpenGLShader::Geometry, ":/shaders/shader.geom");
    program->link();
    program->bind();


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

    if(scene.isReady())
    {
        // diffuse map attaching
        {
            glActiveTexture(GL_TEXTURE0);
            this->diffuseMap->bind();
            glUniform1i(program->uniformLocation("diffuseMap"), 0);
        }
        // displacement map attaching
        {
            glActiveTexture(GL_TEXTURE1);
            this->displacementMap->bind();
            glUniform1i(program->uniformLocation("displacementMap"), 1);
        }
        // normal map attaching
        {
            glActiveTexture(GL_TEXTURE2);
            this->normalMap->bind();
            glUniform1i(program->uniformLocation("normalMap"), 2);
        }
        // specular map attaching
        {
            glActiveTexture(GL_TEXTURE3);
            this->specularMap->bind();
            glUniform1i(program->uniformLocation("specularMap"), 3);
        }
        vao.bind();
        glDrawArrays(GL_PATCHES, 0, sizeof(scene.vertices) / sizeof(scene.vertices[0]));
        this->diffuseMap->release();
        this->displacementMap->release();
        this->normalMap->release();
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
}

void Window::addDiffuse(QImage &diffuseMap)
{   
    scene.addDiffuseMap(diffuseMap, *program);
    this->diffuseMap = new QOpenGLTexture(scene.getDiffuse().mirrored());
    this->diffuseMap->setMinificationFilter(QOpenGLTexture::Linear);
    this->diffuseMap->setMagnificationFilter(QOpenGLTexture::Linear);
}

void Window::addNormal(QImage &normalMap)
{
    scene.addNormalMap(normalMap, *program);
    this->normalMap = new QOpenGLTexture(scene.getNormal().mirrored());
    this->normalMap->setMinificationFilter(QOpenGLTexture::Linear);
    this->normalMap->setMagnificationFilter(QOpenGLTexture::Linear);
}

void Window::addSpecular(QImage &specularMap)
{
    scene.addSpecularMap(specularMap, *program);
    this->specularMap = new QOpenGLTexture(scene.getSpecular().mirrored());
    this->specularMap->setMinificationFilter(QOpenGLTexture::Linear);
    this->specularMap->setMagnificationFilter(QOpenGLTexture::Linear);
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
