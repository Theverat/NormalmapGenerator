#include "graphicsview.h"

GraphicsView::GraphicsView(QGraphicsScene *scene, QWidget *parent) : QGraphicsView(scene, parent)
{
}

GraphicsView::GraphicsView(QWidget *parent) : QGraphicsView(parent)
{
}

void GraphicsView::dragEnterEvent(QDragEnterEvent* event) {
    QGraphicsView::dragEnterEvent(event);
}

void GraphicsView::dragMoveEvent(QDragMoveEvent* event) {
    QGraphicsView::dragMoveEvent(event);
}

void GraphicsView::dropEvent(QDropEvent* event) {
    QGraphicsView::dropEvent(event);

    if(event->mimeData()->hasUrls()) {
        QList<QUrl> urls = event->mimeData()->urls();
        if(urls.size() == 1) {
            emit singleImageDropped(urls.at(0));
        }
        else {
            emit multipleImagesDropped(urls);
        }
    }
}
