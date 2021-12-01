#include "mainwindow.h"

MainWindow::MainWindow(int desktop_width, int desktop_heigh, Core *programm_core, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    desktop_dots_per_percent_x = desktop_width/100;
    desktop_dots_per_percent_y = desktop_heigh/100;
    desktop_zero_x = (desktop_width - desktop_dots_per_percent_x * 100)/2;
    desktop_zero_y = (desktop_heigh - desktop_dots_per_percent_y * 100)/2;

    ui->setupUi(this);
    core = programm_core;  

    about_labor = new Button(this, true,
                             desktop_zero_x + desktop_dots_per_percent_x * 2,
                             desktop_zero_y + desktop_dots_per_percent_y * 2,
                             desktop_dots_per_percent_x * 8,
                             desktop_dots_per_percent_y * 8);
    connect(about_labor, SIGNAL(clicked()), SLOT(output_dialog_window()));
    start = new Button(this, true,
                       desktop_zero_x + desktop_dots_per_percent_x * 42,
                       desktop_zero_y + desktop_dots_per_percent_y * 48,
                       desktop_dots_per_percent_x * 16,
                       desktop_dots_per_percent_y * 15);
    start->changeColor("#868686", "#595959");
    connect(start, SIGNAL(clicked()), core, SLOT(start()));
    stop = new Button(this, true,
                      desktop_zero_x + desktop_dots_per_percent_x * 42,
                      desktop_zero_y + desktop_dots_per_percent_y * 64,
                      desktop_dots_per_percent_x * 16,
                      desktop_dots_per_percent_y * 15);
    stop->changeColor("#868686", "#595959");
    connect(stop, SIGNAL(clicked()), core, SLOT(stop()));

    language_button_mapper = new QSignalMapper(this);
    bool add_language_flag;
    if(core->config_file->lab_info["add_language"] == "-")
        add_language_flag = false;
    else
        add_language_flag = true;
    Button *add_language = new Button(this, add_language_flag,
                                  desktop_zero_x + desktop_dots_per_percent_x * 81,
                                  desktop_zero_y + desktop_dots_per_percent_y * 2,
                                  desktop_dots_per_percent_x * 5,
                                  desktop_dots_per_percent_y * 8);
    add_language->changeText(core->config_file->lab_info["abbreviation"]);
    connect(add_language, SIGNAL(clicked()), language_button_mapper, SLOT(map()));
    language_button_mapper->setMapping(add_language, "Add");
    Button *language_RU = new Button(this, true,
                                     desktop_zero_x + desktop_dots_per_percent_x * 87,
                                     desktop_zero_y + desktop_dots_per_percent_y * 2,
                                     desktop_dots_per_percent_x * 5,
                                     desktop_dots_per_percent_y * 8);
    language_RU->changeText("RU");
    connect(language_RU, SIGNAL(clicked()), language_button_mapper, SLOT(map()));
    language_button_mapper->setMapping(language_RU, "RU");
    Button *language_EN = new Button(this, true,
                                     desktop_zero_x + desktop_dots_per_percent_x * 93,
                                     desktop_zero_y + desktop_dots_per_percent_y * 2,
                                     desktop_dots_per_percent_x * 5,
                                     desktop_dots_per_percent_y * 8);
    language_EN->changeText("EN");
    connect(language_EN, SIGNAL(clicked()), language_button_mapper, SLOT(map()));
    language_button_mapper->setMapping(language_EN, "EN");
    connect(language_button_mapper, SIGNAL(mapped(QString)), this, SLOT(changeLanguage(QString)));

    mode_button_mapper = new QSignalMapper(this);
    for(int i=0; i<core->mode_counter; ++i)
    {
        if(i%2 == 0)
        {
            massive_buttons[i] = new Button(this, core->massive_of_mode[i]->mode_settings["enable"].toInt(),
                                            desktop_zero_x + desktop_dots_per_percent_x * 2,
                                            desktop_zero_y + desktop_dots_per_percent_y * (16 + 16*(i/2)),
                                            desktop_dots_per_percent_x * 18,
                                            desktop_dots_per_percent_y * 15);
        }
        else
        {
            massive_buttons[i] = new Button(this, core->massive_of_mode[i]->mode_settings["enable"].toInt(),
                                            desktop_zero_x + desktop_dots_per_percent_x * 22,
                                            desktop_zero_y + desktop_dots_per_percent_y * (16 + 16*(i/2)),
                                            desktop_dots_per_percent_x * 18,
                                            desktop_dots_per_percent_y * 15);
        }
        connect(massive_buttons[i], SIGNAL(clicked()), mode_button_mapper, SLOT(map()));
        mode_button_mapper->setMapping(massive_buttons[i], i);
    }
    connect(mode_button_mapper, SIGNAL(mapped(int)), core, SLOT(mode(int)));

    QString path = QDir::currentPath();
    for(int i=0; i<6; ++i)
    {
        massive_blocks[i] = new Block(this,
                                      desktop_zero_x + desktop_dots_per_percent_x * (2 + 6*i),
                                      desktop_zero_y + desktop_dots_per_percent_y * 85,
                                      desktop_dots_per_percent_x * 5,
                                      desktop_dots_per_percent_y * 13);
        massive_blocks[i]->picture->changePicture(massive_blocks_path[i]);
    }

    massive_pictures[TIME] = new Picture(this,
                                      desktop_zero_x + desktop_dots_per_percent_x * 42,
                                      desktop_zero_y + desktop_dots_per_percent_y * 1,
                                      desktop_dots_per_percent_x * 16,
                                      desktop_dots_per_percent_y * 10); //часы
    massive_pictures[LAB_NAME] = new Picture(this,
                                      desktop_zero_x + desktop_dots_per_percent_x * 60,
                                      desktop_zero_y + desktop_dots_per_percent_y * 2,
                                      desktop_dots_per_percent_x * 8,
                                      desktop_dots_per_percent_y * 8); //название лаборатории
    massive_pictures[SERIAL_NUMBER] = new Picture(this,
                                      desktop_zero_x + desktop_dots_per_percent_x * 69,
                                      desktop_zero_y + desktop_dots_per_percent_y * 2,
                                      desktop_dots_per_percent_x * 8,
                                      desktop_dots_per_percent_y * 8); //серийный номер лаборатории
    massive_pictures[CHOICE_OF_MODE] = new Picture(this,
                                      desktop_zero_x + desktop_dots_per_percent_x * 2,
                                      desktop_zero_y + desktop_dots_per_percent_y * 12,
                                      desktop_dots_per_percent_x * 38,
                                      desktop_dots_per_percent_y * 4); //выбор режима
    massive_pictures[SCHEME_OF_CONNECTION] = new Picture(this,
                                      desktop_zero_x + desktop_dots_per_percent_x * 60,
                                      desktop_zero_y + desktop_dots_per_percent_y * 12,
                                      desktop_dots_per_percent_x * 38,
                                      desktop_dots_per_percent_y * 4); //схема подключения
    massive_pictures[BLOCKS] = new Picture(this,
                                      desktop_zero_x + desktop_dots_per_percent_x * 2,
                                      desktop_zero_y + desktop_dots_per_percent_y * 81,
                                      desktop_dots_per_percent_x * 38,
                                      desktop_dots_per_percent_y * 4); //блокировки
    massive_pictures[CLUES] = new Picture(this,
                                      desktop_zero_x + desktop_dots_per_percent_x * 43,
                                      desktop_zero_y + desktop_dots_per_percent_y * 81,
                                      desktop_dots_per_percent_x * 55,
                                      desktop_dots_per_percent_y * 4); //подсказки
    massive_pictures[PLACE_FOR_STATE] = new Picture(this,
                                      desktop_zero_x + desktop_dots_per_percent_x * 11,
                                      desktop_zero_y + desktop_dots_per_percent_y * 2,
                                      desktop_dots_per_percent_x * 27,
                                      desktop_dots_per_percent_y * 8); //поле для состояния
    massive_pictures[PLACE_FOR_SCHEME] = new Picture(this,
                                      desktop_zero_x + desktop_dots_per_percent_x * 60,
                                      desktop_zero_y + desktop_dots_per_percent_y * 16,
                                      desktop_dots_per_percent_x * 38,
                                      desktop_dots_per_percent_y * 63); //поле для схемы подключения
    massive_pictures[PLACE_FOR_CLUES] = new Picture(this,
                                      desktop_zero_x + desktop_dots_per_percent_x * 43,
                                      desktop_zero_y + desktop_dots_per_percent_y * 85,
                                      desktop_dots_per_percent_x * 55,
                                      desktop_dots_per_percent_y * 13); //поле для подсказок
    massive_pictures[HIGH_VOLTAGE] = new Picture(this,
                                      desktop_zero_x + desktop_dots_per_percent_x * 42,
                                      desktop_zero_y + desktop_dots_per_percent_y * 12,
                                      desktop_dots_per_percent_x * 16,
                                      desktop_dots_per_percent_y * 36); //поле для значка высокого напряжения
    massive_pictures[LAB_NAME]->changeText(core->config_file->lab_info["name"], Qt::AlignVCenter, 16);
    massive_pictures[SERIAL_NUMBER]->changeText("№ " + core->config_file->lab_info["serial_number"], Qt::AlignVCenter, 16);

    changeLanguage(core->default_language);

    ui_timer = new QTimer();
    connect(ui_timer, SIGNAL(timeout()), this, SLOT(timer_alarm()));
    ui_timer->start(1000);

    connect(core, SIGNAL(startRepaint()), this, SLOT(paintStart()));
    connect(core, SIGNAL(stopRepaint()), this, SLOT(paintStop()));
    connect(core, SIGNAL(buttonsRepaint(bool)), this, SLOT(paintButtons(bool)));
    connect(core, SIGNAL(blocksRepaint(QBitArray)), this, SLOT(paintBlocks(QBitArray)));
    connect(core, SIGNAL(pixmapRepaint(QString,int)), this, SLOT(paintPixmap(QString,int)));
    connect(core, SIGNAL(pixmapRewrite(QString,int)), this, SLOT(writePixmap(QString,int)));
}

MainWindow::~MainWindow()
{
    delete ui;

    core->stop_pressed = 0;
    core->start_enable = 0;
    core->stop();

    delete core;
}

void MainWindow::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);

    QColor grey(0x69,0x69,0x69);
    QColor white(0xff,0xff,0xff);
    painter.setPen(QPen(QBrush(grey), 2.0));
    painter.setBrush(QBrush(white));

    painter.drawRoundedRect(desktop_zero_x + desktop_dots_per_percent_x * 1,
                            desktop_zero_y + desktop_dots_per_percent_y * 1,
                            desktop_dots_per_percent_x * 40,
                            desktop_dots_per_percent_y * 10,
                            core->radius, core->radius);
    painter.drawRoundedRect(desktop_zero_x + desktop_dots_per_percent_x * 42,
                            desktop_zero_y + desktop_dots_per_percent_y * 1,
                            desktop_dots_per_percent_x * 16,
                            desktop_dots_per_percent_y * 10,
                            core->radius, core->radius);
    painter.drawRoundedRect(desktop_zero_x + desktop_dots_per_percent_x * 59,
                            desktop_zero_y + desktop_dots_per_percent_y * 1,
                            desktop_dots_per_percent_x * 40,
                            desktop_dots_per_percent_y * 10,
                            core->radius, core->radius);
    painter.drawRoundedRect(desktop_zero_x + desktop_dots_per_percent_x * 1,
                            desktop_zero_y + desktop_dots_per_percent_y * 12,
                            desktop_dots_per_percent_x * 40,
                            desktop_dots_per_percent_y * 68,
                            core->radius, core->radius);
    painter.drawRoundedRect(desktop_zero_x + desktop_dots_per_percent_x * 59,
                            desktop_zero_y + desktop_dots_per_percent_y * 12,
                            desktop_dots_per_percent_x * 40,
                            desktop_dots_per_percent_y * 68,
                            core->radius, core->radius);
    painter.drawRoundedRect(desktop_zero_x + desktop_dots_per_percent_x * 1,
                            desktop_zero_y + desktop_dots_per_percent_y * 81,
                            desktop_dots_per_percent_x * 40,
                            desktop_dots_per_percent_y * 18,
                            core->radius, core->radius);
    painter.drawRoundedRect(desktop_zero_x + desktop_dots_per_percent_x * 42,
                            desktop_zero_y + desktop_dots_per_percent_y * 81,
                            desktop_dots_per_percent_x * 57,
                            desktop_dots_per_percent_y * 18,
                            core->radius, core->radius);
}

void MainWindow::paintStart()
{
    if(core->start_enable)
    {
        if(core->start_pressed)
            start->changeColor("#868686", "#595959");
        else
            start->changeColor("#66FF00", "#45AC00");
    }
    else
        start->changeColor("#868686", "#595959");
}

void MainWindow::paintStop()
{
    if(core->stop_pressed)
        stop->changeColor("#868686", "#595959");
    else
        stop->changeColor("#FF0000", "#AC0000");
}

void MainWindow::paintButtons(bool enable)
{
    if(enable)
    {
        for(int i=0; i<core->mode_counter; ++i)
        {
            if(i == core->mode_number)
                massive_buttons[i]->changeColor("#00A8FF", "#005CB1");
            else
                massive_buttons[i]->changeColor("#D2D2D2", "#A4A4A4");
        }
    }
    else
    {
        for(int i=0; i<core->mode_counter; ++i)
        {
            if(i == core->mode_number)
                massive_buttons[i]->changeColor("#00A8FF", "#005CB1");
            else
                massive_buttons[i]->changeColor("#868686", "#595959");
        }
    }
}

void MainWindow::paintBlocks(QBitArray read_result)
{
    for(int i=0; i<16; ++i)
    {
        if(read_result[i])
        {
            switch (i)
            {
                case Core::Blocks::VOLTAGE:
                    massive_blocks[0]->changeColor("#1FBE00", "#199600");
                    core->blocks_flags.setBit(i, false);
                    break;
                case Core::Blocks::CURRENT:
                    massive_blocks[1]->changeColor("#1FBE00", "#199600");
                    core->blocks_flags.setBit(i, false);
                    break;
                case Core::Blocks::DOORS:
                    massive_blocks[2]->changeColor("#1FBE00", "#199600");
                    core->blocks_flags.setBit(i, false);
                    break;
                case Core::Blocks::EMERGENCY_BUTTON:
                    massive_blocks[3]->changeColor("#1FBE00", "#199600");
                    core->blocks_flags.setBit(i, false);
                    break;
                case Core::Blocks::GND:
                    massive_blocks[4]->changeColor("#1FBE00", "#199600");
                    core->blocks_flags.setBit(i, false);
                    break;
                case Core::Blocks::WORK_GND:
                    massive_blocks[5]->changeColor("#1FBE00", "#199600");
                    core->blocks_flags.setBit(i, false);
                    break;
            }
        }
        else
        {
            switch (i)
            {
                case Core::Blocks::VOLTAGE:
                    massive_blocks[0]->changeColor("#CE0030", "#960023");
                    if((core->blocks_mask[i]) && (!core->blocks_flags[i]))
                    {
                        core->log_file->sendMessage("Сработала блокировка " + core->int_to_blocks[i]);
                        core->blocks_flags.setBit(i, true);
                    }
                    break;
                case Core::Blocks::CURRENT:
                    massive_blocks[1]->changeColor("#CE0030", "#960023");
                    if((core->blocks_mask[i]) && (!core->blocks_flags[i]))
                    {
                        core->log_file->sendMessage("Сработала блокировка " + core->int_to_blocks[i]);
                        core->blocks_flags.setBit(i, true);
                    }
                    break;
                case Core::Blocks::DOORS:
                    massive_blocks[2]->changeColor("#CE0030", "#960023");
                    if((core->blocks_mask[i]) && (!core->blocks_flags[i]))
                    {
                        core->log_file->sendMessage("Сработала блокировка " + core->int_to_blocks[i]);
                        core->blocks_flags.setBit(i, true);
                    }
                    break;
                case Core::Blocks::EMERGENCY_BUTTON:
                    massive_blocks[3]->changeColor("#CE0030", "#960023");
                    if((core->blocks_mask[i]) && (!core->blocks_flags[i]))
                    {
                        core->log_file->sendMessage("Сработала блокировка " + core->int_to_blocks[i]);
                        core->blocks_flags.setBit(i, true);
                    }
                    break;
                case Core::Blocks::GND:
                    massive_blocks[4]->changeColor("#CE0030", "#960023");
                    if((core->blocks_mask[i]) && (!core->blocks_flags[i]))
                    {
                        core->log_file->sendMessage("Сработала блокировка " + core->int_to_blocks[i]);
                        core->blocks_flags.setBit(i, true);
                    }
                    break;
                case Core::Blocks::WORK_GND:
                    massive_blocks[5]->changeColor("#CE0030", "#960023");
                    if((core->blocks_mask[i]) && (!core->blocks_flags[i]))
                    {
                        core->log_file->sendMessage("Сработала блокировка " + core->int_to_blocks[i]);
                        core->blocks_flags.setBit(i, true);
                    }
                    break;
            }
        }
    }
}

void MainWindow::paintPixmap(QString path, int number)
{
//    QString t_path = QDir::currentPath().append("/" + path);
    massive_pictures[number]->changePicture(path);
}

void MainWindow::writePixmap(QString text, int number)
{
    massive_pictures[number]->changeText(text, Qt::AlignVCenter, 12);
}

void MainWindow::output_dialog_window()
{
    if(!core->errors_flag)
    {
        dialog = new DialogWindow(core->config_file->ui_translation["name"][core->default_language] + "\n" + core->config_file->lab_info["name"] + "\n\n" +
                                  core->config_file->ui_translation["serial_number"][core->default_language] + "\n" + core->config_file->lab_info["serial_number"] + "\n\n" +
                                  core->config_file->ui_translation["date_of_production"][core->default_language] + "\n" + core->config_file->lab_info["date_of_production"] + "\n\n" +
                                  core->config_file->ui_translation["controller_version"][core->default_language] + "\n" + core->config_file->lab_info["controller_version"] + "\n\n" +
                                  core->config_file->ui_translation["default_language"][core->default_language] + "\n" + time_language);
        dialog->setWindowFlags(dialog->windowFlags() & ~Qt::WindowContextHelpButtonHint);
        dialog->setWindowTitle("О лаборатории");
        dialog->exec();
        delete dialog;
    }
}

void MainWindow::timer_alarm()
{
    massive_pictures[0]->changeText(QTime::currentTime().toString("hh:mm:ss"), Qt::AlignCenter, 30);
}

void MainWindow::changeLanguage(QString language)
{
    if(!core->start_pressed)
    {
        if(language == "RU")
        {
            core->default_language = "RU";
            time_language = "Русский";
            core->config_file->changeDefaultLanguage("Русский");
            core->log_file->sendMessage("Установлен язык Русский");
        }
        else if(language == "EN")
        {
            core->default_language = "EN";
            time_language = "English";
            core->config_file->changeDefaultLanguage("Английский");
            core->log_file->sendMessage("Установлен язык Английский");
        }
        else
        {
            core->default_language = "Add";
            time_language = core->config_file->lab_info["add_language"];
            core->config_file->changeDefaultLanguage("Дополнительный");
            core->log_file->sendMessage("Установлен язык " + core->config_file->lab_info["add_language"]);
        }

        about_labor->changeText(core->config_file->ui_translation["about_labor"][language], 12);
        start->changeText(core->config_file->ui_translation["start"][language]);
        stop->changeText(core->config_file->ui_translation["stop"][language]);

        for(int i=0; i<core->mode_counter; ++i)
        {
            massive_buttons[i]->changeText(core->massive_of_mode[i]->mode_settings["name_" + language]);
        }
        massive_pictures[CHOICE_OF_MODE]->changeText(core->config_file->ui_translation["choice_of_mode"][language], Qt::AlignVCenter);
        massive_pictures[SCHEME_OF_CONNECTION]->changeText(core->config_file->ui_translation["scheme_of_connection"][language], Qt::AlignVCenter);
        massive_pictures[BLOCKS]->changeText(core->config_file->ui_translation["blocks"][language], Qt::AlignVCenter);
        massive_pictures[CLUES]->changeText(core->config_file->ui_translation["clues"][language], Qt::AlignVCenter);
        if(!core->first_iteration)
            massive_pictures[PLACE_FOR_CLUES]->changeText(core->massive_of_mode[core->mode_number]->mode_settings["clue_" + language], Qt::AlignVCenter, 12);
        if(core->start_enable)
            massive_pictures[PLACE_FOR_STATE]->changeText(core->config_file->ui_translation["device_is_ready"][language], Qt::AlignVCenter, 12);
        if(core->errors_flag)
            massive_pictures[PLACE_FOR_STATE]->changeText(core->config_file->blocks_messages[core->modbus_computer->err_to_qstr[core->copy_modbus_answer.second]][language], Qt::AlignVCenter, 12);
    }
}
