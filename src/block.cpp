#include "block.h"

Block::Block(QWidget *parent, int top_left_x, int top_left_y, int width, int heigh)
    : QLabel(parent)
{
    picture = new Picture(parent, top_left_x, top_left_y, width, heigh);
    this->setGeometry(top_left_x, top_left_y, width, heigh); //определяем месторасположение
}

Block::~Block()
{

}

void Block::paintEvent(QPaintEvent *event)
{
    QPainter painter(this); //отрисовываем фон
    painter.setBrush(QBrush(QColor(local_background_color)));
    painter.setPen(QPen(QBrush(QColor(local_border_color)), 1.5));
    painter.drawRoundedRect(radius/2, radius/2, size().width()-radius, size().height()-radius, radius, radius);
    QLabel::paintEvent(event);
}

void Block::changeColor(QString background_color, QString border_color)
{
    local_background_color = background_color; //устанавливаем параметры цвета
    local_border_color = border_color;
    this->repaint();
}
