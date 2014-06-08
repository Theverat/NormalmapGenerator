#ifndef QUEUEITEM_H
#define QUEUEITEM_H

#include <QListWidgetItem>
#include <QUrl>

class QueueItem : public QListWidgetItem
{
public:
    QueueItem(QUrl imageUrl, const QString &text, QListWidget *view, int type);
    QUrl getUrl();

private:
    QUrl imageUrl;

};

#endif // QUEUEITEM_H
