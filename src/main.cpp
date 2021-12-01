#include "mainwindow.h"
#include "core.h"

#include <QApplication>
#include <QDebug>
#include <QDesktopWidget>
//#include <QObject>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QDesktopWidget *desktop = QApplication::desktop();

    Core *programm_core = new Core(); //инициализация ядра
    MainWindow ui(desktop->width(), desktop->height(), programm_core); //инициализация UI
    if(programm_core->errors_flag != true)
    {
        ui.showFullScreen();
        return app.exec();
    }
    else
    {
        app.exit();
    }
}
