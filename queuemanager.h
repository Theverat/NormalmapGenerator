#ifndef QUEUEMANAGER_H
#define QUEUEMANAGER_H

#include <QListWidget>
#include <QDir>

class QueueManager
{
public:
    QueueManager(QListWidget *queue);
    void processQueue(QDir exportPath);

public slots:
    void stopProcessingQueue();

private:
    QListWidget *queue;
};

#endif // QUEUEMANAGER_H
