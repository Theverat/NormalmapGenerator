#include "light.h"

Light::Light()
{
    rotator = QQuaternion();
    Light::ambientIntensity = 0.3f;
    Light::color = QVector3D(1.0f, 1.0f, 1.0f);
    Light::diffuseIntensity = 0.7f;
    Light::direction = QVector3D(1.0f, 1.0f, 1.0f);
    Light::specularPower = 1.0f;
    Light::materialShines = 100.0f;
}


void Light::rotateLeft()
{
    rotate(QQuaternion::fromAxisAndAngle(QVector3D(0.0f, 1.0f, 0.0f), 5.0f));
    world.setToIdentity();
    world.rotate(rotator.inverted());
    direction = world * direction;
}

void Light::rotateDown()
{
    rotate(QQuaternion::fromAxisAndAngle(QVector3D(1.0f, 0.0f, 0.0f), 5.0f));
    world.setToIdentity();
    world.rotate(rotator);
    direction = world * direction;
}

void Light::rotateUp()
{
    rotate(QQuaternion::fromAxisAndAngle(QVector3D(1.0f, 0.0f, 0.0f), 5.0f));
    world.setToIdentity();
    world.rotate(rotator.inverted());
    direction = world * direction;
}

void Light::rotateRight()
{
    rotate(QQuaternion::fromAxisAndAngle(QVector3D(0.0f, 1.0f, 0.0f), 5.0f));
    world.setToIdentity();
    world.rotate(rotator);
    direction = world * direction;
}
