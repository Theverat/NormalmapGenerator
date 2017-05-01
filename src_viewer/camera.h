#ifndef CAMERA_H
#define CAMERA_H

#include <QVector3D>
#include <QMatrix4x4>
#include <QQuaternion>

class Camera
{
public:
    Camera();

    void rotateX(float angle);
    void rotateY(float angle);
    void zoomIn();
    void zoomOut();

    const QMatrix4x4 &toMatrix();
    QVector3D getPosition() { return position; }
private:
    bool dirty;
    float xAngle;
    float yAngle;
    float radius;

    QVector3D position;

    QMatrix4x4 world;
};

#endif // CAMERA_H
