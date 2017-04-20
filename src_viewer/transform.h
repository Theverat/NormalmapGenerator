#ifndef TRANSFORM_H
#define TRANSFORM_H

#include <QVector3D>
#include <QMatrix4x4>
#include <QQuaternion>

class Transform
{
public:
    Transform();

    void setTranslation(QVector3D translation);
    void setScale(float scale);
    void setRotation(float xRot, float yRot);

    QVector3D forward() const;
    QVector3D up() const;
    QVector3D right() const;

    const QMatrix4x4 &toMatrix();
private:
    bool dirty;

    QVector3D translation;
    QVector3D scale;
    QQuaternion rotation;

    QMatrix4x4 transform;

    static const QVector3D LOCAL_FORWARD;
    static const QVector3D LOCAL_UP;
    static const QVector3D LOCAL_RIGHT;
};

#endif // TRANSFORM_H
