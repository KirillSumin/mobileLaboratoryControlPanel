#include "button.h"

Button::Button(QWidget *parent, bool enable, int top_left_x, int top_left_y, int width, int heigh)
    :QPushButton(parent)
{
    local_enable = enable;
    this->setGeometry(top_left_x, top_left_y, width, heigh); //определяем место расположение
    this->changeColor("#D2D2D2", "#A4A4A4"); //устанавливаем параметры стиля
}

Button::~Button()
{

}

void Button::paintEvent(QPaintEvent *event)
{
    if(local_enable)
    {
        QPainter painter(this); //отрисовываем картинку при ее наличии
        painter.drawPixmap((size().width()-scaled_picture.width())/2, (size().height()-scaled_picture.height())/2, scaled_picture);
        QPushButton::paintEvent(event); //отрисовываем кнопку
    }
}

void Button::changePicture(QString picture_path)
{
    if(!picture_path.isEmpty()) //определяем параметры картинки
    {
        QPixmap picture(picture_path);
        scaled_picture = picture.scaled(size(), Qt::KeepAspectRatio);
    }
    this->repaint();
}

void Button::changeText(QString text, int font_size)
{
    if(!text.isEmpty()) //устанавливаем параметры надписи
    {
        this->setStyle(new ButtonStyle());
        this->setFont(QFont(local_font, font_size));
        this->setText(text);
    }
    this->repaint();
}

void Button::changeColor(QString background_color, QString border_color)
{
    this->setStyleSheet("background: " + background_color + "; border-color: " + border_color + ";"
                        "border-width: 3; border-style: outset;"
                        "border-radius: " + QString::number(radius) + ";");
}

////////////////////

ButtonStyle::ButtonStyle()
    : QProxyStyle()
{

}

void ButtonStyle::drawItemText(QPainter *painter, const QRect &rect, int flags, const QPalette &pal, bool enabled, const QString &text, QPalette::ColorRole textRole) const
{
    flags |= Qt::TextWordWrap;
    QProxyStyle::drawItemText(painter, rect, flags, pal, enabled, text, textRole);
}
