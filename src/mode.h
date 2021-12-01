#ifndef MODE_H
#define MODE_H

#include <QStringList>
#include <QMap>
//#include <QDebug>

class Mode
{
public:
    explicit Mode(QStringList list, int mode_counter);

    QMap<QString, QString> mode_settings;
    QString error_text = "Режим успешно записан";
    bool error_flag = false;

private:
    const int length = 24;
    const int name_number = 0;
    const int clue_number = 17;
};

#endif // MODE_H
