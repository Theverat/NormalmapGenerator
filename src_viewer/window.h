#ifndef WINDOW_H
#define WINDOW_H
#include <QImage>
#include <QMatrix4x4>
#include <QGroupBox>

#include <QOpenGLFunctions>
#include <QOpenGLWidget>
#include <QOpenGLTexture>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>

#include "src_viewer/scene.h"




class QOpenGLShaderProgram;

class Window : public QOpenGLWidget,
               protected QOpenGLFunctions
{
    Q_OBJECT

// OpenGL Events
public:
    ~Window();
    Window(QGroupBox*);
    void initializeGL();
    void resizeGL(int width, int height);
    void paintGL();
    void teardownGL();
    // to control from main program
    // texture adding
    void addDisplacement(QImage &displacementMap);
    void addDiffuse(QImage &diffuseMap);
    void addNormal(QImage &normalMap);
    void addSpecular(QImage &specularMap);

    void setDepthValue(float newDepth);
    void setPartitionFrequency(int newPartFreq);
    void setRotating(bool value);
protected:
    void keyPressEvent(QKeyEvent* event);
    void mousePressEvent(QMouseEvent* pe);
    void mouseMoveEvent(QMouseEvent* pe);
    void mouseReleaseEvent(QMouseEvent*);
protected slots:
    void update();
private:
    Scene scene;
    Light light;

    float cameraSpeed = 0.1f;
    float rotatingSpeed = 100.0f;

    bool areWeRotating = false;

    // renderer properties
    float depth = 3.0f;
    int partitionFrequency = 20;

    // Input stuff
    bool pressed;
    QPoint ptrMousePosition;

    // OpenGL State Information
    QOpenGLBuffer bufferForVertices;
    QOpenGLVertexArrayObject vao;
    QOpenGLShaderProgram *program;

    GLuint u_lightColor;
    GLuint u_lightAmbI;
    GLuint u_lightDifI;
    GLuint u_lightDir;
    GLuint u_lightSpec;
    GLuint u_lightMatShiness;

    GLuint u_modelToWorld;
    GLuint u_worldToCamera;
    GLuint u_cameraToView;

    GLuint u_cameraPosition;

    GLuint u_depth;
    GLuint u_partFreq;

    QOpenGLTexture *diffuseMap;
    QOpenGLTexture *displacementMap;
    QOpenGLTexture *normalMap;
    QOpenGLTexture *specularMap;

};

#endif // WINDOW_H
