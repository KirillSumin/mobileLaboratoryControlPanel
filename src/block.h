#ifndef BLOCK_H
#define BLOCK_H

#include "picture.h"
#include "core.h"

#include <QLabel>
//#include <QPaintEvent>
#include <QPainter>
//#include <QDebug>

class Core;
class Picture;

class Block: public QLabel
{
    Q_OBJECT

public:
    Block( QWidget *parent = nullptr, int top_left_x = 0, int top_left_y = 0, int width = 100, int heigh = 100);
    ~Block();
    void changeColor(QString background_color, QString border_color);

    Picture *picture;

protected:
    void paintEvent(QPaintEvent *event);

private:
//    Core *core;
    qreal radius = 8.0;
    QString local_background_color = "#868686";
    QString local_border_color = "#595959";
};

#endif // BLOCK_H
