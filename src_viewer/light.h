#ifndef LIGHT_H
#define LIGHT_H
#include <QVector3D>
#include <QQuaternion>
#include <QMatrix4x4>

class Light
{
private:
    QVector3D color;
    float ambientIntensity;
    QVector3D direction;
    float diffuseIntensity;
    float specularPower;
    float materialShines;

    QMatrix4x4 world;
    QQuaternion rotator;
public:
    // mutators
    void inline setColor(const QVector3D& color)            { this->color = color; }
    void inline setAmbientIntensity(const float& ambInt)    { this->ambientIntensity = ambInt; }
    void inline setDirection(const QVector3D& direction)    { this->direction = direction; }
    void inline setDiffuseIntensity(const float& difInt)    { this->diffuseIntensity = difInt; }
    void inline setSpecularPower(const float& specPower)    { this->specularPower = specPower; }
    void inline setMaterialShines(const float& matShines)    { this->materialShines = matShines; }


    // accessors
    inline const QVector3D& Color()         { return color; }
    inline const float& AmbientIntensity()  { return ambientIntensity; }
    QVector3D& Direction()           { return direction; }
    inline const float& DiffuseIntensity()  { return diffuseIntensity; }
    inline const float& SpecularPower()     { return specularPower; }
    inline const float& MatertialShines()   { return materialShines; }
    Light();

    void rotate(const QQuaternion &dr) { rotator = dr; }
    void rotateLeft();
    void rotateRight();
    void rotateUp();
    void rotateDown();
};

#endif // LIGHT_H
