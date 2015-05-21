#ifndef LISTWIDGET_H
#define LISTWIDGET_H

#include <QListWidget>
#include <QUrl>
#include <QDropEvent>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QMimeData>

class ListWidget : public QListWidget
{
    Q_OBJECT

public:
    explicit ListWidget(QWidget *parent = 0);
    void dragEnterEvent(QDragEnterEvent* event);
    void dragMoveEvent(QDragMoveEvent* event);
    void dropEvent(QDropEvent *event);

signals:
    void singleImageDropped(QUrl url);
    void multipleImagesDropped(QList<QUrl> urls);

public slots:

};

#endif // LISTWIDGET_H
