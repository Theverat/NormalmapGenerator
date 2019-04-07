#ifndef CLICKABLELABEL_H
#define CLICKABLELABEL_H

#include <QLabel>

class ClickableLabel : public QLabel
{
    Q_OBJECT
public:
    ClickableLabel(QWidget* parent = 0) : QLabel(parent) {}
    ClickableLabel(const QString& text = "", QWidget* parent = 0) : QLabel(parent) {
        setText(text);
    }
    ~ClickableLabel() {}
    
signals:
    void clicked();
    
protected:
    void mousePressEvent(QMouseEvent *) {
        emit clicked(); 
    }
};

#endif // CLICKABLELABEL_H
