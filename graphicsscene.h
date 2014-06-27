#ifndef MYQGRAPHICSSCENE_H
#define MYQGRAPHICSSCENE_H

#include <QGraphicsScene>

class GraphicsScene : public QGraphicsScene
{
public:
    GraphicsScene(QObject *parent = 0);
    GraphicsScene(const QRectF &sceneRect, QObject *parent = 0);
    GraphicsScene(qreal x, qreal y, qreal width, qreal height, QObject *parent = 0);
    void dragEnterEvent(QGraphicsSceneDragDropEvent *event);
    void dragMoveEvent(QGraphicsSceneDragDropEvent* event);
    void dropEvent(QGraphicsSceneDragDropEvent* event);
};

#endif // MYQGRAPHICSSCENE_H
