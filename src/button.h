#ifndef BUTTON_H
#define BUTTON_H

#include "picture.h"
#include "core.h"

#include <QPushButton>
#include <QPainter>
#include <QProxyStyle>

class Core;
class Picture;

class Button: public QPushButton
{
    Q_OBJECT

public:
    Button(QWidget *parent = nullptr, bool enable = true, int top_left_x = 0, int top_left_y = 0, int width = 100, int heigh = 100);
    ~Button();
    void changePicture(QString picture_path);
    void changeText(QString text, int font_size = 16);
    void changeColor(QString background_color, QString border_color);

    bool local_enable;

protected:
    void paintEvent(QPaintEvent *event);

private:
    qreal radius = 8.0;
    QString local_font = "Times";
    QPixmap scaled_picture;
};

////////////////////

class ButtonStyle : public QProxyStyle
{
public:
    ButtonStyle();
    virtual void drawItemText(QPainter *painter, const QRect &rect,
        int flags, const QPalette &pal, bool enabled,
        const QString &text, QPalette::ColorRole textRole) const;

private:
    Q_DISABLE_COPY(ButtonStyle)
};

#endif // BUTTON_H
