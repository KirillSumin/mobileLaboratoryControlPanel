#ifndef PICTURE_H
#define PICTURE_H

#include <QLabel>
#include <QPainter>
#include <QDir>
//#include <QDebug>

class Picture: public QLabel
{
    Q_OBJECT

public:
    Picture(QWidget *parent = nullptr, int top_left_x = 0, int top_left_y = 0, int width = 100, int heigh = 100);
    ~Picture();
    void changePicture(QString picture_path);
    void changeText(QString text, Qt::AlignmentFlag state = Qt::AlignCenter, int font_size = 16);

protected:
    void paintEvent(QPaintEvent *event);

private:
    QString local_font = "Times";
    QPixmap scaled_picture;
};

#endif // PICTURE_H
