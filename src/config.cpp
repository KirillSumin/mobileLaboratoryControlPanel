#include "config.h"

Config::Config(Core *programm_core, const QString file_name)
{
    core = programm_core;
    file = new QFile(file_name); //определяем конфигурационный файл
    file->open(QIODevice::ReadOnly);
    if (!file->exists()) //проверка на наличие конфигурационного файла
    {
        error_flag = true;
        error_text = "Отсутствует конфигурационный файл!";
    }
    else //считываем конфигурационный файл
    {
        core->log_file->sendMessage("Конфигурационный файл успешно открыт");
        error_text = read();
    }
}

Config::~Config()
{

}

void Config::changeDefaultLanguage(QString new_language)
{
    file->open(QIODevice::WriteOnly |QIODevice::Truncate);

    QTextStream output_stream(file); //устанавливаем исходящий поток данных
    output_stream.setCodec(QTextCodec::codecForName("UTF-8"));

    QStringList::iterator iterator_1 = data_copy.begin();
    int string_counter = 0;
    while(iterator_1 != data_copy.end())
    {
        if(string_counter == 13)
        {
           output_stream << new_language << "\n";
           (++iterator_1);
        }
        else
        {
            output_stream << *iterator_1 << "\n";
            (++iterator_1);
        }
        (++string_counter);
    }

    file->close();
}

QString Config::read()
{
    QTextStream input_stream(file); //устанавливаем входящий поток данных
    input_stream.setCodec(QTextCodec::codecForName("UTF-8"));

    QStringList::iterator iterator_1;
    QStringList::iterator iterator_2;
    QString time_string;
    QMap<QString, QString> time_translation;
    QList<QString> keys;

    QStringList data; //записываем данные из файла для дальнейшей работы
    while (!input_stream.atEnd())
    {
        data << input_stream.readLine();
    }
    data_copy = data;
    data.removeAll(QString("")); //очищаем от пустых строк
    if (data.empty()) //проверяем наличие данных
    {
        error_flag = true;
        return "Данные в конфигурационном файле отсутствуют!";
    }
    else
        core->log_file->sendMessage("Считываем данные из конфигурационного файла");
    iterator_1 = data.begin();

    while((*iterator_1 != "-----") && (iterator_1 != data.end())) //считываем информацию о лаборатории
    {
        time_string = *iterator_1;
        lab_info.insert(time_string,  *(++iterator_1));
        (++iterator_1);
    }
    keys = lab_info.keys(); //проверяем информацию о лаборатории
    if(keys.length() != lab_info_length)
    {
        error_flag = true;
        return "Информация о лаборатории повреждена!";
    }
    else
        core->log_file->sendMessage("Информация о лаборатории успешно записана");
    (++iterator_1);

    while((*iterator_1 != "-----") && (iterator_1 != data.end())) //считываем настройки modbus
    {
        time_string = *iterator_1;
        modbus_settings.insert(time_string,  *(++iterator_1));
        (++iterator_1);
    }
    keys = modbus_settings.keys(); //проверяем целостность настроек modbus
    if(keys.length() != modbus_settings_length)
    {
        error_flag = true;
        return "Настройки Modbus повреждены!";
    }
    else
        core->log_file->sendMessage("Настройки Modbus успешно записаны");
    (++iterator_1);

    iterator_2 = list_translation_RU_EN.begin();
    while((*iterator_1 != "-----") && (iterator_1 != data.end())) //считываем перевод интерфейса на дополнительный язык
    {
        QString string_for_translation = *iterator_1;
        time_translation.insert("RU", *(iterator_2));
        time_translation.insert("EN", *(++iterator_2));
        time_string = *(++iterator_1);
        time_translation.insert(time_string,  *(++iterator_1));
        ui_translation.insert(string_for_translation, time_translation);
        time_translation.clear();
        (++iterator_1);
        (++iterator_2);
    }
    keys = ui_translation.keys(); //проверяем целостность перевода интерфейса на дополнительный язык
    if(keys.length() != ui_translation_length)
    {
        error_flag = true;
        return "Данные перевода интерфейса на дополнительный язык повреждены!";
    }
    else
        core->log_file->sendMessage("Данные перевода интерфейса на дополнительный язык успешно записаны");
    (++iterator_1);

    while((*iterator_1 != "-----") && (iterator_1 != data.end())) //считываем уведомления блокировок
    {
        QString block_name = *iterator_1;
        for(int i=0; i<3; i++)
        {
            time_string = *(++iterator_1);
            time_translation.insert(time_string, *(++iterator_1));
        }
        blocks_messages.insert(block_name, time_translation);
        time_translation.clear();
        (++iterator_1);
    }
    keys = blocks_messages.keys(); //проверяем целостность уведомлений блокировок
    if(keys.length() != blocks_messages_length)
    {
        error_flag = true;
        return "Уведомления блокировок повреждены!";
    }
    else
        core->log_file->sendMessage("Уведомления блокировок успешно записаны");
    (++iterator_1);

    QStringList time_mode_list; //считываем данные по настройке каждого режима в элемент core->massive_of_mode
    while (iterator_1 != data.end())
    {
        if (*iterator_1 == "-----")
        {
            core->massive_of_mode[core->mode_counter] = new Mode(time_mode_list, core->mode_counter);
            time_mode_list.clear();
            if(core->massive_of_mode[core->mode_counter]->error_flag)
            {
                error_flag = core->massive_of_mode[core->mode_counter]->error_flag;
                return core->massive_of_mode[core->mode_counter]->error_text;
            }
            else
                core->log_file->sendMessage(core->massive_of_mode[core->mode_counter]->error_text);
            (++core->mode_counter);
        }
        else
            time_mode_list << *(iterator_1);
        (++iterator_1);
    }
    core->massive_of_mode[core->mode_counter] = new Mode(time_mode_list, core->mode_counter);
    if(core->massive_of_mode[core->mode_counter]->error_flag)
    {
        error_flag = core->massive_of_mode[core->mode_counter]->error_flag;
        return core->massive_of_mode[core->mode_counter]->error_text;
    }
    else
        core->log_file->sendMessage(core->massive_of_mode[core->mode_counter]->error_text);
    (++core->mode_counter);

    file->close(); //закрываем конфигурационный файл
    return "Успешное прочтение конфигурационного файла";
}
