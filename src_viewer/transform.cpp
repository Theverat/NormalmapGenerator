#include "transform.h"

const QVector3D Transform::LOCAL_FORWARD(0.0f, 0.0f, 1.0f);
const QVector3D Transform::LOCAL_UP(0.0f, 1.0f, 0.0f);
const QVector3D Transform::LOCAL_RIGHT(1.0f, 0.0f, 0.0f);

Transform::Transform()
{
    dirty = true;
    setTranslation(QVector3D(0.0f, 0.0f, 0.0f));
    setScale(1.0f);
}

void Transform::setTranslation(QVector3D translation)
{
    dirty = true;
    this->translation = translation;
}

void Transform::setScale(float scale)
{
    dirty = true;
    this->scale = QVector3D(scale, scale, scale);
}

void Transform::setRotation(float xRot, float yRot)
{
    dirty = true;
    this->rotation = QQuaternion::fromAxisAndAngle(1.0f, 0.0f, 0.0f, xRot) *
            QQuaternion::fromAxisAndAngle(0.0f, 1.0f, 0.0f, yRot);
}

QVector3D Transform::forward() const
{
    return rotation.rotatedVector(LOCAL_FORWARD);
}

QVector3D Transform::up() const
{
    return rotation.rotatedVector(LOCAL_UP);
}

QVector3D Transform::right() const
{
    return rotation.rotatedVector(LOCAL_RIGHT);
}

const QMatrix4x4 &Transform::toMatrix()
{
    if(dirty)
    {
        transform.translate(translation);
        transform.scale(scale);
        transform.rotate(rotation);
        dirty = false;
    }
    return transform;
}




