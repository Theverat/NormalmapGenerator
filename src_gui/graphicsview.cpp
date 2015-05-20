/********************************************************************************
 *   Copyright (C) 2015 by Simon Wendsche                                       *
 *                                                                              *
 *   This file is part of NormalmapGenerator.                                   *
 *                                                                              *
 *   NormalmapGenerator is free software; you can redistribute it and/or modify *
 *   it under the terms of the GNU General Public License as published by       *
 *   the Free Software Foundation; either version 3 of the License, or          *
 *   (at your option) any later version.                                        *
 *                                                                              *
 *   NormalmapGenerator is distributed in the hope that it will be useful,      *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of             *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the              *
 *   GNU General Public License for more details.                               *
 *                                                                              *
 *   You should have received a copy of the GNU General Public License          *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.      *
 *                                                                              *
 *   Sourcecode: https://github.com/Theverat/NormalmapGenerator                 *
 ********************************************************************************/

#include "graphicsview.h"
#include <QMimeData>
#include <QDropEvent>
#include <iostream>

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

void GraphicsView::mouseReleaseEvent(QMouseEvent *event) {
    QGraphicsView::mouseReleaseEvent(event);
    
    if(event->button() == Qt::RightButton) {
        //rightclick -> reset image scale to 1:1
        emit rightClick();
    }
    else if(event->button() == Qt::MiddleButton) {
        //middle click -> fit image to view
        emit middleClick();
    }
}

void GraphicsView::wheelEvent(QWheelEvent *event) {
    if(event->delta() > 0) {
        emit zoomIn();
    }
    else {
        emit zoomOut();
    }
}
