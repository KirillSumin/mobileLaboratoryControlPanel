#ifndef LOGFILE_H
#define LOGFILE_H


#include <QFile>
#include <QTextStream>
#include <QTextCodec>
#include <QDateTime>
//#include <QDebug>

class LogFile
{
public:
    LogFile(const QString file_name = nullptr);
    ~LogFile();
    void sendMessage(QString message);

    bool error_flag = false;
    QString error_text = "No Errors";

private:
    QFile *file;
    QTextStream *output_stream;

};

#endif // LOGFILE_H
