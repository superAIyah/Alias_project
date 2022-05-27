#include "configwindow.h"
#include "ui_configwindow.h"
#include "Setting.h"
#include "gamebuilder.h"

#include <iostream>
#include <QDebug>

ConfigWindow::ConfigWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ConfigWindow)
{
    ui->setupUi(this);
    gameWindow = new GameWindow();
}

ConfigWindow::~ConfigWindow()
{
    delete ui;
}

void ConfigWindow::on_findGameButton_clicked()
{
    //ui->BoxMode->setVisible(!ui->BoxMode->isVisible());

    //Получаем все RadioButtons у каждого типа настройки
    QList<QRadioButton *> length_radios = ui->BoxLength->findChildren<QRadioButton *>(); // длина раунда
    QList<QRadioButton *> team_cnt_radios = ui->BoxTeamCnt->findChildren<QRadioButton *>(); // количество команд
    QList<QRadioButton *> team_sz_radios = ui->BoxTeamSz->findChildren<QRadioButton *>(); // размер команды

    // Определяем настройки соотв. классам
    Setting round = Setting(length_radios);
    Setting team_cnt = Setting(team_cnt_radios);
    Setting team_sz = Setting(team_sz_radios);

    // Список со всеми настройками (Полиморфизм)
    std::vector<ISetting *> configs = {&team_sz, &team_cnt, &round};

    // Создаем класс, собирающий все настройки воедино
    GameBuilder configs_builder(configs);
    GameConfig game_config;
    game_config = configs_builder.CollectSettings();
    qDebug() << game_config.GetSettings().c_str();

    // <-- вызов функции поиска игры клиента  -->

    // ... Когда игра найдена начинается игра ...
    // как вызывать?
    gameWindow->show();
    this->hide();
}
