#ifndef CONFIG_H
#define CONFIG_H

//#include "mode.h"
#include "core.h"

#include <QFile>
#include <QTextStream>
#include <QTextCodec>
//#include <iostream>
//#include <QDebug>
#include <QMap>

class Core;

class Config
{
public:
    Config(Core *programm_core = nullptr, const QString file_name = nullptr);
    ~Config();
    void changeDefaultLanguage(QString new_language);

    QMap<QString, QString> lab_info;
    QMap<QString, QString> modbus_settings;
    QMap<QString, QMap<QString, QString>> ui_translation;
    QMap<QString, QMap<QString, QString>> blocks_messages;

    bool error_flag = false;
    QString error_text = "Успешное прочтение конфигурационного файла";

private:
    QString read();

    QFile *file;
    QStringList data_copy;
    QStringList list_translation_RU_EN = {"О лаборатории", "About lab",
                                        "ПУСК", "START",
                                        "СТОП", "STOP",
                                        "Выбор режима", "Choice of mode",
                                        "Схема подключения", "Scheme of connection",
                                        "Блокировки", "Blocks",
                                        "Подсказки", "Clues",
                                        "Название лаборатории", "Name of the lab",
                                        "Серийный номер", "Serial number",
                                        "Дата изготовления", "Date of production",
                                        "Верси ПО контроллера", "Controller software version",
                                        "Язык", "Language",
                                        "Устройство готово к работе", "Device is ready",
                                        "Работа", "Work"};
    Core *core;

    const int lab_info_length = 7;
    const int modbus_settings_length = 7;
    const int ui_translation_length = 14;
    const int blocks_messages_length = 14;

};

#endif // CONFIG_H
