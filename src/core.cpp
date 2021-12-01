#include "core.h"

Core::Core(QObject *parent)
    : QObject(parent)
{
    QString path = QDir::currentPath(); //объявляем путь до папки со всеми файлами

    log_file = new LogFile(path + "/logfile.txt"); //инициализируем файл логирования
    if(log_file->error_flag)
    {
        infoOutput(log_file->error_text);
        errors_flag = true;
    }
    if(!errors_flag)
    {
        config_file = new Config(this, path + "/config/config_file.txt"); //инициализируем конфигурационный файл
        if(config_file->error_flag) //проверяем на наличие ошибок при инициализации
        {
            log_file->sendMessage(config_file->error_text);
            infoOutput(config_file->error_text);
            errors_flag = true;
        }
        else
        {
            log_file->sendMessage(config_file->error_text);
            if(config_file->lab_info["default_language"] == "Русский")
                default_language = "RU";
            if(config_file->lab_info["default_language"] == "Английский")
                default_language = "EN";
            if(config_file->lab_info["default_language"] == "Дополнительный")
                default_language = "Add";
        }
        if(!errors_flag)
        {
            modbus_computer = new ModbusMaster(this);
            modbus_computer->setModbusLabParameters(config_file->modbus_settings["com_port"],
                                                    config_file->modbus_settings["baud_rate"],
                                                    config_file->modbus_settings["stop_bits"],
                                                    config_file->modbus_settings["data_bits"],
                                                    config_file->modbus_settings["parity"],
                                                    config_file->modbus_settings["response_time"],
                                                    config_file->modbus_settings["adress"]);
//            ModbusMaster::ModbusError connection_answer = ModbusMaster::ModbusError::NO_ERROR;
            ModbusMaster::ModbusError connection_answer = modbus_computer->connectModbus();
            if(connection_answer != ModbusMaster::ModbusError::NO_ERROR)
            {
                log_file->sendMessage(config_file->blocks_messages[modbus_computer->err_to_qstr[connection_answer]]["RU"]);
                infoOutput(config_file->blocks_messages[modbus_computer->err_to_qstr[connection_answer]][default_language]);
                errors_flag = true;
            }
            else
            {
                QString info[2] = {"2103", "0"};
//                QPair<QString[2], ModbusMaster::ModbusError> ID_answer = qMakePair(info, ModbusMaster::ModbusError::NO_ERROR);
                QPair<QString[2],ModbusMaster::ModbusError> ID_answer = modbus_computer->readID();
                if(ID_answer.second != ModbusMaster::ModbusError::NO_ERROR)
                {
                    log_file->sendMessage(config_file->blocks_messages[modbus_computer->err_to_qstr[ID_answer.second]]["RU"]);
                    infoOutput(config_file->blocks_messages[modbus_computer->err_to_qstr[ID_answer.second]][default_language]);
                    errors_flag = true;
                }
                else
                {
                    if(ID_answer.first[0] != config_file->lab_info["serial_number"])
                    {
                        log_file->sendMessage(config_file->blocks_messages["INVALID_SERIAL_NUMBER"]["RU"]);
                        infoOutput(config_file->blocks_messages["INVALID_SERIAL_NUMBER"][default_language]);
                        errors_flag = true;
                    }
                    else if(ID_answer.first[1] != config_file->lab_info["controller_version"])
                    {
                        log_file->sendMessage(config_file->blocks_messages["INVALID_CONTROLLER_VERSION"]["RU"]);
                        infoOutput(config_file->blocks_messages["INVALID_CONTROLLER_VERSION"][default_language]);
                        errors_flag = true;
                    }
                }
            }
            if(!errors_flag)
            {
                core_timer = new QTimer();
                connect(core_timer, SIGNAL(timeout()), this, SLOT(timer()));
                core_timer->start(1000);
            }
        }
    }
}

Core::~Core()
{
//    delete modbus_computer;
    delete log_file;
}

void Core::timer()
{
    if(!modbus_answer_flag)
    {
        if(!first_time_flag)
        {
            modbus_answer_flag = true;
    //        QPair<ModbusMaster::Status, ModbusMaster::ModbusError> modbus_status = qMakePair(ModbusMaster::Status::s_MANUAL, ModbusMaster::ModbusError::NO_ERROR);
            QPair<ModbusMaster::Status, ModbusMaster::ModbusError> modbus_status = modbus_computer->readStatus();
            modbus_answer_flag = false;
            if (modbus_status.second == ModbusMaster::ModbusError::NO_ERROR)
            {
                if(modbus_status.first != ModbusMaster::s_MANUAL)
                {
                    modbus_answer_flag = true;
        //                qDebug() << "Выставлен режим работы s_MANUAL";
                    modbus_computer->writeCommand(ModbusMaster::c_MANUAL);
                    modbus_answer_flag = false;
                    first_time_flag = true;
                }
            }
            else
                errorOutput(modbus_status.second);
        }

        modbus_answer_flag = true;
//        QBitArray modbus_inputs = QBitArray(16, false);
//        modbus_inputs.setBit(2,true);
//        modbus_inputs.setBit(3,true);
//        QPair<QBitArray, ModbusMaster::ModbusError> modbus_answer = qMakePair(modbus_inputs, ModbusMaster::ModbusError::NO_ERROR);
        QPair<QBitArray, ModbusMaster::ModbusError> modbus_answer = modbus_computer->readRegister(input_register_adress);
        modbus_answer_flag = false;
        copy_modbus_answer = modbus_answer;
        if(modbus_answer.second == ModbusMaster::ModbusError::NO_ERROR)
        {
            if(errors_flag)
            {
                errors_flag = false;
                emit pixmapRewrite(" ", MainWindow::Pixmaps::PLACE_FOR_STATE);
            }
            emit blocksRepaint(modbus_answer.first);

            if ((blocks_mask == (blocks_mask & modbus_answer.first)) && (blocks_mask != QBitArray(16, false)))
            {
                if(start_pressed == 0)
                    emit pixmapRewrite(config_file->ui_translation["device_is_ready"][default_language], MainWindow::Pixmaps::PLACE_FOR_STATE);
                if(start_enable == 0)
                {
                    modbus_answer_flag = true;
        //            ModbusMaster::ModbusError modbus_work_result = ModbusMaster::ModbusError::NO_ERROR;
//                    qDebug() << "Зажглась зеленая лампочка";
                    ModbusMaster::ModbusError modbus_write_green_led_answer = modbus_computer->writeBit(green_led_pin, true);
                    modbus_answer_flag = false;

                    if(modbus_write_green_led_answer != ModbusMaster::ModbusError::NO_ERROR)
                        errorOutput(modbus_write_green_led_answer);

                    start_enable = 1;
//                    stop_pressed = 0;
                    start_pressed = 0;

                    emit startRepaint();

                    log_file->sendMessage("Все блокировки устранены");
                }
            }
            else if ((blocks_mask != (blocks_mask & modbus_answer.first)) && (blocks_mask != QBitArray(16, false)))
            {
                QString time_string;
                int string_counter = 0;
                int string_number = 2;
                bool flag = true;
                for(int i=0; i<16; ++i)
                {
                    if((blocks_mask[i] == 1) && (modbus_answer.first[i] == 0))
                    {
                        if(string_counter < string_number)
                        {
                            if(flag)
                                time_string = config_file->blocks_messages[int_to_blocks[i]][default_language];
                            else
                                time_string = time_string + "\n" + config_file->blocks_messages[int_to_blocks[i]][default_language];
                        }
                        flag = false;
                    }
                }
                if(start_enable == 1)
                {
                    modbus_answer_flag = true;
//                    ModbusMaster::ModbusError modbus_work_result = ModbusMaster::ModbusError::NO_ERROR;
//                    qDebug() << "Погасла зеленая лампочка";
                    ModbusMaster::ModbusError modbus_write_green_led_answer = modbus_computer->writeBit(green_led_pin, false);
                    modbus_answer_flag = false;
                    if(modbus_write_green_led_answer != ModbusMaster::ModbusError::NO_ERROR)
                        errorOutput(modbus_write_green_led_answer);

                    start_enable = 0;
//                    stop_pressed = 0;
                    emit startRepaint();
                    stop();
                 }
                 emit pixmapRewrite(time_string, MainWindow::Pixmaps::PLACE_FOR_STATE);
            }
        }
        else
            errorOutput(modbus_answer.second);
    }
}

void Core::mode(int number)
{
    if((!errors_flag) && (number != mode_number) && (!start_pressed))
    {
        first_iteration = false;
        mode_number = number;
        log_file->sendMessage("Выбран режим " + QString::number(number + 1));
        if ((massive_of_mode[number]->mode_settings["enable"].toInt() == 1) && (start_pressed == 0))
        {
            int length;
            int step;
            QString time_string;
            char* symbol = const_cast<char*>(" ");

            work_actions.fill(false);
            blocks_mask.fill(false);

            length = massive_of_mode[number]->mode_settings["modbus_outputs"].length();
            time_string = massive_of_mode[number]->mode_settings["modbus_outputs"];
            step = 0;
            for (int i = 0; i < length; ++i)
            {
                if (massive_of_mode[number]->mode_settings["modbus_outputs"][i] != *symbol)
                    step++;
                else
                {
                    work_actions.setBit(time_string.left(step).toInt() - output_register_adress * 16);
                    time_string = massive_of_mode[number]->mode_settings["modbus_outputs"].right(length - 1 - i);
                    step = 0;
                }
            }
            work_actions.setBit(time_string.left(step).toInt() - output_register_adress * 16);

            length = massive_of_mode[number]->mode_settings["modbus_inputs"].length();
            time_string = massive_of_mode[number]->mode_settings["modbus_inputs"];
            step = 0;
            for (int i = 0; i < length; ++i)
            {
                if (massive_of_mode[number]->mode_settings["modbus_inputs"][i] != *symbol)
                    step++;
                else
                {
                    blocks_mask.setBit(time_string.left(step).toInt() - input_register_adress * 16);
                    time_string = massive_of_mode[number]->mode_settings["modbus_inputs"].right(length - 1 - i);
                    step = 0;
                }
            }
            blocks_mask.setBit(time_string.left(step).toInt() - input_register_adress * 16);

            exe_path = massive_of_mode[number]->mode_settings["exe_path"];

            emit buttonsRepaint(true);
            emit pixmapRepaint(massive_of_mode[number]->mode_settings["image_path"], MainWindow::Pixmaps::PLACE_FOR_SCHEME);
            emit pixmapRewrite(massive_of_mode[mode_number]->mode_settings["clue_" + default_language], MainWindow::Pixmaps::PLACE_FOR_CLUES);
        }
    }
}

void Core::start()
{
    if((!modbus_answer_flag) && (!errors_flag))
    {
        modbus_answer_flag = true;
//        QPair<ModbusMaster::Status, ModbusMaster::ModbusError> modbus_status = qMakePair(ModbusMaster::Status::s_MANUAL, ModbusMaster::ModbusError::NO_ERROR);
        QPair<ModbusMaster::Status, ModbusMaster::ModbusError> modbus_status = modbus_computer->readStatus();
        modbus_answer_flag = false;
        if (modbus_status.second == ModbusMaster::ModbusError::NO_ERROR)
        {
            if(modbus_status.first != ModbusMaster::s_MANUAL)
            {
                modbus_answer_flag = true;
//                qDebug() << "Выставлен режим работы s_MANUAL";
                modbus_computer->writeCommand(ModbusMaster::c_MANUAL);
                modbus_answer_flag = false;
            }
            if ((start_enable == 1) && (start_pressed == 0))
            {
                stop_pressed = 0;
                timer();
                if((start_enable == 1) && (stop_pressed == 0))
                {
                    modbus_answer_flag = true;
//                    ModbusMaster::ModbusError modbus_work_answer = ModbusMaster::ModbusError::NO_ERROR;
//                    int work_actions_length = work_actions.size();
//                    for(int i=0; i<work_actions_length; ++i)
//                    {
//                        if(work_actions[i])
//                            ModbusMaster::ModbusError modbus_work_answer = modbus_computer->writeBit(i + output_register_adress * 16, true);
//                    }
                    ModbusMaster::ModbusError modbus_write_register_anwer = modbus_computer->writeRegister(output_register_adress, work_actions);
                    modbus_answer_flag = false;
                    if(modbus_write_register_anwer == ModbusMaster::ModbusError::NO_ERROR)
                    {
                        if((exe_path != "-") && (!errors_flag))
                        {
                            exe_file = new QProcess(this);
                            exe_file->setProgram(exe_path);
                            exe_file->startDetached();
                        }
                    }
                    else
                        errorOutput(modbus_write_register_anwer);
                }
                if((!errors_flag) && (stop_pressed == 0))
                {
                    modbus_answer_flag = true;
    //                ModbusMaster::ModbusError modbus_work_answer = ModbusMaster::ModbusError::NO_ERROR;
    //                qDebug() << "Погасла зеленая лампочка";
                    ModbusMaster::ModbusError modbus_write_green_led_answer = modbus_computer->writeBit(green_led_pin, false);
                    modbus_answer_flag = false;
                    if(modbus_write_green_led_answer != ModbusMaster::ModbusError::NO_ERROR)
                        errorOutput(modbus_write_green_led_answer);

                    modbus_answer_flag = true;
    //                QBitArray modbus_outputs = QBitArray(16, true);
    //                QPair<QBitArray, ModbusMaster::ModbusError> modbus_answer = qMakePair(modbus_outputs, ModbusMaster::ModbusError::NO_ERROR);
                    QPair<QBitArray, ModbusMaster::ModbusError> modbus_answer = modbus_computer->readRegister(output_register_adress);
                    modbus_answer_flag = false;
                    if(modbus_answer.second == ModbusMaster::ModbusError::NO_ERROR)
                    {
                        if (modbus_answer.first[red_led_pin - output_register_adress * 16])
                               emit pixmapRepaint("high_voltage.png", MainWindow::Pixmaps::HIGH_VOLTAGE);
                    }
                    else
                        errorOutput(modbus_answer.second);

                    start_pressed = 1;
                    emit startRepaint();
                    emit stopRepaint();
                    emit buttonsRepaint(false);
                    emit pixmapRewrite(config_file->ui_translation["work"][default_language], MainWindow::Pixmaps::PLACE_FOR_STATE);

                    log_file->sendMessage("ПУСК");
                    log_file->sendMessage("Включены pin-ы: " + massive_of_mode[mode_number]->mode_settings["modbus_outputs"]);
                }
//                else
//                    stop_pressed = 1;
            }
        }
        else
            errorOutput(modbus_status.second);
    }
}

void Core::stop()
{
    if(!stop_pressed)
    {
        if((!modbus_answer_flag) && (!errors_flag))
        {
            modbus_answer_flag = true;
    //        ModbusMaster::ModbusError modbus_work_answer = ModbusMaster::ModbusError::NO_ERROR;
            ModbusMaster::ModbusError modbus_write_register_answer = modbus_computer->writeRegister(output_register_adress, QBitArray(16, false));
            modbus_answer_flag = false;
            if(modbus_write_register_answer == ModbusMaster::ModbusError::NO_ERROR)
            {
                if(start_pressed != 0)
                    log_file->sendMessage("СТОП");

                start_pressed = 0;
                stop_pressed = 1;

                emit startRepaint();
                emit stopRepaint();
                emit buttonsRepaint(true);
                emit pixmapRewrite(" ", MainWindow::Pixmaps::PLACE_FOR_STATE);

                if(start_enable)
                {
                    modbus_answer_flag = true;
        //            ModbusMaster::ModbusError modbus_work_result = ModbusMaster::ModbusError::NO_ERROR;
        //            qDebug() << "Зажглась зеленая лампочка";
                    ModbusMaster::ModbusError modbus_write_green_led_answer = modbus_computer->writeBit(green_led_pin, true);
                    modbus_answer_flag = false;
                    if(modbus_write_green_led_answer != ModbusMaster::ModbusError::NO_ERROR)
                        errorOutput(modbus_write_green_led_answer);
                }

                modbus_answer_flag = true;
        //        QBitArray modbus_outputs = QBitArray(16, false);
        //        QPair<QBitArray, ModbusMaster::ModbusError> modbus_answer = qMakePair(modbus_outputs, ModbusMaster::ModbusError::NO_ERROR);
                QPair<QBitArray, ModbusMaster::ModbusError> modbus_answer = modbus_computer->readRegister(output_register_adress);
                modbus_answer_flag = false;
                if(modbus_answer.second == ModbusMaster::ModbusError::NO_ERROR)
                {
                    if(!modbus_answer.first[red_led_pin - output_register_adress * 16])
                        emit pixmapRepaint("", MainWindow::Pixmaps::HIGH_VOLTAGE);
                }
                else
                    errorOutput(modbus_answer.second);
            }
            else
                errorOutput(modbus_write_register_answer);
        }
    }
}

void Core::errorOutput(ModbusMaster::ModbusError error)
{
    if(!errors_flag)
    {
        log_file->sendMessage(config_file->blocks_messages[modbus_computer->err_to_qstr[error]]["RU"]);
        emit pixmapRewrite(config_file->blocks_messages[modbus_computer->err_to_qstr[error]][default_language], MainWindow::Pixmaps::PLACE_FOR_STATE);
        emit blocksRepaint(QBitArray(16,false));
    }
    infoOutput(config_file->blocks_messages[modbus_computer->err_to_qstr[error]][default_language]);
    errors_flag = true;
}

void Core::delay()
{
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
}

void Core::infoOutput(QString message)
{
    if(!info_window)
    {
        info_window = new DialogWindow(message);
        QDesktopWidget *desktop = QApplication::desktop();
        info_window->setGeometry((desktop->width()/2) - 600, (desktop->height()/2) - 300, 1200, 600);
        info_window->setWindowFlags(info_window->windowFlags() & ~Qt::WindowContextHelpButtonHint);
        info_window->setWindowTitle("ERROR");
        info_window->exec();
        delete info_window;
        info_window = nullptr;
    }
}


