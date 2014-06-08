#include "queueitem.h"

QueueItem::QueueItem(QUrl imageUrl, const QString &text, QListWidget *view, int type)
    : QListWidgetItem(text, view, type)
{
    this->imageUrl = imageUrl;
}

QUrl QueueItem::getUrl() {
    return imageUrl;
}
