#include "listwidget.h"

#include <iostream>

ListWidget::ListWidget(QWidget *parent) :
    QListWidget(parent)
{
}

void ListWidget::dragEnterEvent(QDragEnterEvent* event) {
    event->accept();
}

void ListWidget::dragMoveEvent(QDragMoveEvent* event) {
    event->accept();
}

void ListWidget::dropEvent(QDropEvent *event) {
    event->accept();

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
