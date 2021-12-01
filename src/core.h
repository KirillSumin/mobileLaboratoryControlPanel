#ifndef CORE_H
#define CORE_H

#include "config.h"
#include "logfile.h"
#include "mode.h"
#include "mainwindow.h"
#include "modbusmaster.h"
#include "dialogwindow.h"

#include <QObject>
#include <QBitArray>
#include <QSignalMapper>
#include <QDir>
#include <QTimer>
#include <QPair>
#include <QProcess>
#include <QCoreApplication>
#include <QDesktopWidget>
#include <QApplication>
#include <QDebug>
class MainWindow;
class DialogWindow;
class Config;

class Core: public QObject
{
    Q_OBJECT

public:
    Core(QObject *parent = nullptr);
    ~Core();

    enum Blocks
    {
        VOLTAGE = 6,
        CURRENT = 7,
        DOORS = 5,
        EMERGENCY_BUTTON = 4,
        GND = 2,
        WORK_GND = 3
    };
    QMap<int, QString> int_to_blocks = {{6, "VOLTAGE"},
                                        {7, "CURRENT"},
                                        {5, "DOORS"},
                                        {4, "EMERGENCY_BUTTON"},
                                        {2, "GND"},
                                        {3, "WORK_GND"}};

    LogFile *log_file;
    Config *config_file;
    ModbusMaster *modbus_computer;

    Mode *massive_of_mode[8];
    int mode_counter = 0; //число режимов
    int mode_number = 100; //какой режим нажать
    QBitArray work_actions = QBitArray(16, false);
    QBitArray blocks_mask = QBitArray(16, false);
    QBitArray blocks_flags = QBitArray(16, false);

    bool start_enable = 0;
    bool start_pressed = 0;
    bool stop_pressed = 1;

    int output_register_adress = 1;
    int input_register_adress = 0;
    int red_led_pin = 16;
    int green_led_pin = 17;

    QString default_language;
    QString exe_path = "";
    QProcess *exe_file;
    DialogWindow *info_window = nullptr;
    QPair<QBitArray, ModbusMaster::ModbusError> copy_modbus_answer;
    qreal radius = 8.0;
    bool first_iteration = true;
    bool errors_flag = false;
    bool modbus_answer_flag = false;
    bool first_time_flag = false;

private:
    void infoOutput(QString message);
    void errorOutput(ModbusMaster::ModbusError error);
    void delay();

    QTimer *core_timer;

public slots:
    void timer();
    void mode(int number);
    void start();
    void stop();

signals:
    void startRepaint();
    void stopRepaint();
    void buttonsRepaint(bool);
    void blocksRepaint(QBitArray);
    void pixmapRepaint(QString, int);
    void pixmapRewrite(QString, int);

};

#endif // CORE_H
