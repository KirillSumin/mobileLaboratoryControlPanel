#include "mode.h"

Mode::Mode(QStringList mode_list, int mode_counter)
{
    if (mode_list.length() != length) //проверяем целостность данных по настройке режима
    {
        error_flag = true;
        error_text = "Данные настроек режимов повреждены!";
    }
    else //записываем данные по настройке режима в mode_settings
    {
        QStringList::iterator iterator = mode_list.begin();
        QString string_for_translation;
        QString time_string;
        for(int i=0; i<24; ++i)
        {
            if((i == name_number) || (i == clue_number))
            {
                string_for_translation = *iterator;
                (++iterator);
            }
            else
            {
                if((i < name_number + 6) || (i > clue_number)) //clue_number + 6 = length
                {
                    time_string = *iterator;
                    mode_settings.insert(string_for_translation + "_" + time_string, *(++iterator));
                    (++iterator);
                    (++i);
                }
                else
                {
                    time_string = *iterator;
                    mode_settings.insert(time_string, *(++iterator));
                    (++iterator);
                    (++i);
                }
            }
        }
        error_text = "Режим " + QString::number(mode_counter + 1) + " успешно записан";
    }
}
