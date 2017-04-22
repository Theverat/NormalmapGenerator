#include "scene.h"

Scene::Scene()
{
}

void Scene::setCameraTranslation(const QVector3D &translation)
{
    camera.setTranslation(translation);
}

void Scene::setCameraRotation(float xRot, const QVector3D axis)
{
    camera.setRotation(xRot, axis);
}

void Scene::translateCameraBy(float speed, const QVector3D axis)
{
    camera.translateBy(speed, axis);
}

void Scene::setTransformTranslation(const QVector3D &translation)
{
    transform.setTranslation(translation);
}

void Scene::setTransformScale(float scale)
{
    transform.setScale(scale);
}

void Scene::setTransformRotation(float xRot, float yRot)
{
    transform.setRotation(xRot, yRot);
}

void Scene::initializeSceneOpenGLFunctions()
{
    initializeOpenGLFunctions();
}

void Scene::changeProjection(int width, int height)
{
    projection.setToIdentity();
    projection.perspective(45.0f, width / float(height), 0.0f, 1000.0f);
}

bool Scene::isReady()
{
    return ready;
}

const QMatrix4x4 &Scene::getModelToWorldMatrix()
{
    return transform.toMatrix();
}

const QMatrix4x4 &Scene::getWorldToCameraMatrix()
{
    return camera.toMatrix();
}

const QMatrix4x4 &Scene::getCameraToViewMatrix()
{
    return projection;
}

void Scene::addDiffuseMap(QImage &diffuseMap, QOpenGLShaderProgram &program)
{
    diffuse = QImage(diffuseMap);
    diffuseMapStored = true;
    ready = checkForReady();
}

void Scene::addDisplacementMap(QImage &displacementMap, QOpenGLShaderProgram &program)
{
    displacement = QImage(displacementMap);
    displacementMapStored = true;
    ready = checkForReady();
}

void Scene::addNormalMap(QImage &normalMap, QOpenGLShaderProgram &program)
{
    normal = QImage(normalMap);
    normalMapStored = true;
    ready = checkForReady();
}

void Scene::addSpecularMap(QImage &specularMap, QOpenGLShaderProgram &program)
{
    specular = QImage(specularMap);
    specularMapStored = true;
    ready = checkForReady();
}

void Scene::calculateVertices()
{
    int count = 0;
    for(int side = 0; side < 6; ++side)
    {
        QVector3D start;
        QVector3D stepU;
        QVector3D stepV;
        switch(side) {
        case 0:
            start = QVector3D(-0.5, -0.5, 0.5);
            stepU = QVector3D(0.1, 0.0, 0.0);
            stepV = QVector3D(0.0, 0.1, 0.0);
            break;
        case 1:
            start = QVector3D(0.5, -0.5, 0.5);
            stepU = QVector3D(0.0, 0.0, -0.1);
            stepV = QVector3D(0.0, 0.1, 0.0);
            break;
        case 2:
            start = QVector3D(0.5, -0.5, -0.5);
            stepU = QVector3D(-0.1, 0.0, 0.0);
            stepV = QVector3D(0.0, 0.1, 0.0);
            break;
        case 3:
            start = QVector3D(-0.5, -0.5, -0.5);
            stepU = QVector3D(0.0, 0.0, 0.1);
            stepV = QVector3D(0.0, 0.1, 0.0);
            break;
        case 4:
            start = QVector3D(-0.5, 0.5, 0.5);
            stepU = QVector3D(0.1, 0.0, 0.0);
            stepV = QVector3D(0.0, 0.0, -0.1);
            break;
        case 5:
            start = QVector3D(-0.5, -0.5, -0.5);
            stepU = QVector3D(0.1, 0.0, 0.0);
            stepV = QVector3D(0.0, 0.0, 0.1);
            break;
        default:
            break;
        }
        for(int i = 0; i < 10; ++i)
        {
            for(int j = 0; j < 10; ++j)
            {
                vertices[count] = Vertex(start + i * stepU + j * stepV);
                vertices[count + 1] = Vertex(start + i * stepU + j * stepV + stepU);
                vertices[count + 2] = Vertex(start + i * stepU + j * stepV + stepV);
                vertices[count + 3] = Vertex(start + i * stepU + j * stepV + stepU + stepV);
                count += 4;
            }
        }
    }
}

bool Scene::checkForReady()
{
    return (diffuseMapStored &&
            displacementMapStored &&
            normalMapStored &&
            specularMapStored);
}











