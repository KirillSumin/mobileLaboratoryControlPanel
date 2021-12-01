#include "logfile.h"

LogFile::LogFile(const QString file_name)
{
    file = new QFile(file_name); //определяем файл логирования
    file->open(QFile::Append | QFile::Text);
    if(!file->exists()) //проверка на наличие файла логирования
    {
        error_flag = true;
        error_text = "Не удалось создать файл логирования!";
    }
    else //начинаем сеанс записи
    {
        output_stream = new QTextStream(file); //создаем исходящий поток данных
        sendMessage("Начало сеанса");
    }
}

LogFile::~LogFile()
{
    sendMessage("Конец сеанса");
    file->close();
}

void LogFile::sendMessage(QString message)
{
    *output_stream << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss ") << ": " << message.toUtf8() << "\n"; //отправляем сообщение в файл логирования
    output_stream->flush();
}
