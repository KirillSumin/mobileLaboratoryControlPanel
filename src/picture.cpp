#include "picture.h"

Picture::Picture(QWidget *parent, int top_left_x, int top_left_y, int width, int heigh)
    : QLabel(parent)
{
    this->setGeometry(top_left_x, top_left_y, width, heigh); //определяем месторасположение
}

Picture::~Picture()
{

}

void Picture::paintEvent(QPaintEvent *event)
{
    QPainter painter(this); //отрисовываем картинку при ее наличии
    painter.drawPixmap((size().width()-scaled_picture.width())/2, (size().height()-scaled_picture.height())/2, scaled_picture);
    QLabel::paintEvent(event);
}

void Picture::changePicture(QString picture_path)
{
    if(!picture_path.isEmpty()) //определяем параметры картинки
    {
        QString t_path = QDir::currentPath().append("/config/img/" + picture_path);
        QPixmap picture(t_path);
        scaled_picture = picture.scaled(size(), Qt::KeepAspectRatio);
    }
    else
        scaled_picture = QPixmap("");
    this->repaint();
}

void Picture::changeText(QString text, Qt::AlignmentFlag state, int font_size)
{
    if(!text.isEmpty()) //устанавливаем параметры надписи
    {
        this->setWordWrap(true);
        this->setAlignment(state);
        this->setFont(QFont(local_font, font_size));
        this->setText(text);
    }
    this->repaint();
}
