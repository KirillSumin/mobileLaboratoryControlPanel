#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "block.h"
#include "button.h"
#include "dialogwindow.h"
#include "picture.h"
#include "config.h"
#include "mode.h"
#include "core.h"
#include "ui_mainwindow.h"

#include <QMainWindow>
#include <QTimer>
#include <QDateTime>
#include <QSignalMapper>
#include <QDesktopWidget>
#include <QApplication>
//#include <QDebug>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE
class Core;
class Config;
class Button;
class Block;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(int desktop_width, int desktop_heigh, Core *programm_core = nullptr, QWidget *parent = nullptr);
    ~MainWindow();

    enum Pixmaps
    {
        TIME = 0,
        LAB_NAME = 1,
        SERIAL_NUMBER = 2,
        CHOICE_OF_MODE = 3,
        SCHEME_OF_CONNECTION = 4,
        PLACE_FOR_SCHEME = 5,
        BLOCKS = 6,
        CLUES = 7,
        PLACE_FOR_CLUES = 8,
        PLACE_FOR_STATE = 9,
        HIGH_VOLTAGE = 10
    };

public slots:
    void paintStart();
    void paintStop();
    void paintButtons(bool enable);
    void paintBlocks(QBitArray read_result);
    void paintPixmap(QString path, int number);
    void writePixmap(QString text, int number);
    void changeLanguage(QString language);
    void output_dialog_window();
    void timer_alarm();

protected:
    void paintEvent(QPaintEvent *event);

private:
    Ui::MainWindow *ui;
    Core *core;
    QTimer *ui_timer;
    DialogWindow *dialog;

    int desktop_dots_per_percent_x;
    int desktop_dots_per_percent_y;
    int desktop_zero_x;
    int desktop_zero_y;

    QSignalMapper *mode_button_mapper;
    QSignalMapper *language_button_mapper;

    Button *about_labor;
    Button *start;
    Button *stop;
    Button *massive_buttons[8];
    Block *massive_blocks[6];
    Picture *massive_pictures[11];

    QString massive_blocks_path[6] = {"U_max.png", "I_max.png", "doors.png", "emergency_button.png", "GND.png", "work_GND.png"};

    QString text_about_labor;
    QString time_language;

};
#endif // MAINWINDOW_H
