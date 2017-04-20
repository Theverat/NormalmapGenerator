#include "camera.h"


const QVector3D Camera::LOCAL_FORWARD(0.0f, 0.0f, -1.0f);
const QVector3D Camera::LOCAL_UP(0.0f, 1.0f, 0.0f);
const QVector3D Camera::LOCAL_RIGHT(1.0f, 0.0f, 0.0f);

Camera::Camera()
{
    dirty = true;
    setTranslation(QVector3D(2.0f, 0.0f, 2.0f));
    setRotation(45.0f, LOCAL_UP);
}

void Camera::setTranslation(const QVector3D &translation)
{
    dirty = true;
    this->translation = translation;
}


void Camera::setRotation(float xRot, const QVector3D axis)
{
    dirty = true;
    this->rotation = QQuaternion::fromAxisAndAngle(axis, xRot)
            * this->rotation;
}

void Camera::translateBy(float speed, const QVector3D axis)
{
    dirty = true;
    translation += speed * axis;
}

QVector3D Camera::forward() const
{
    return rotation.rotatedVector(LOCAL_FORWARD);
}

QVector3D Camera::up() const
{
    return rotation.rotatedVector(LOCAL_UP);
}

QVector3D Camera::right() const
{
    return rotation.rotatedVector(LOCAL_RIGHT);
}

const QMatrix4x4 &Camera::toMatrix()
{
    if(dirty)
    {
        world.setToIdentity();
        world.rotate(rotation.conjugated());
        world.translate(-translation);
        dirty = false;
    }
    return world;
}
