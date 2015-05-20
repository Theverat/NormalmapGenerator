#include "graphicsscene.h"
#include <QGraphicsSceneDragDropEvent>

GraphicsScene::GraphicsScene(QObject *parent)
    : QGraphicsScene(parent)
{
}

GraphicsScene::GraphicsScene(const QRectF &sceneRect, QObject *parent)
    : QGraphicsScene(sceneRect, parent)
{
}

GraphicsScene::GraphicsScene(qreal x, qreal y, qreal width, qreal height, QObject *parent)
    : QGraphicsScene(x, y, width, height, parent)
{
}

void GraphicsScene::dragEnterEvent(QGraphicsSceneDragDropEvent *event) {
}

void GraphicsScene::dragMoveEvent(QGraphicsSceneDragDropEvent *event) {
}

void GraphicsScene::dropEvent(QGraphicsSceneDragDropEvent *event) {
}
