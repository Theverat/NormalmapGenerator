#ifndef CAMERA_H
#define CAMERA_H

#include <QVector3D>
#include <QMatrix4x4>
#include <QQuaternion>

class Camera
{
public:
    Camera();

    void setTranslation(const QVector3D &translation);
    void setRotation(float xRot, const QVector3D axis);
    void translateBy(float speed, const QVector3D axis);

    QVector3D forward() const;
    QVector3D up() const;
    QVector3D right() const;

    QVector3D localForward()    const { return LOCAL_FORWARD; }
    QVector3D localUp()         const { return LOCAL_UP; }
    QVector3D localRight()      const { return LOCAL_RIGHT; }

    const QMatrix4x4 &toMatrix();
    QVector3D getPosition() { return translation; }
private:
    bool dirty;
    QVector3D translation;
    QQuaternion rotation;

    QMatrix4x4 world;

    static const QVector3D LOCAL_FORWARD;
    static const QVector3D LOCAL_UP;
    static const QVector3D LOCAL_RIGHT;
};

#endif // CAMERA_H
