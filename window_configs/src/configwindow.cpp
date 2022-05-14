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
    //Получаем все RadioButtons у каждого типа настройки
    QList<QRadioButton *> teamRadios = ui->BoxMode->findChildren<QRadioButton *>();
    QList<QRadioButton *> levelRadios = ui->BoxLevel->findChildren<QRadioButton *>();
    QList<QRadioButton *> lengthRadios = ui->BoxLength->findChildren<QRadioButton *>();

    // Определяем настройки соотв. классам
    Setting mode = Setting(teamRadios);
    Setting level = Setting(levelRadios);
    Setting round = Setting(lengthRadios);

    // Список со всеми настройками (Полиморфизм)
    std::vector<ISetting *> configs = {&mode, &level, &round};

    // Создаем класс, собирающий все настройки воедино
    GameBuilder configs_builder(configs);
    GameConfig game_config;
    game_config = configs_builder.CollectSettings();
    qDebug() << game_config.GetSettings().c_str();

    // <-- вызов функции поиска игры клиента  -->

    // ... Когда игра найдена начинается игра ...
    gameWindow->show();
    this->hide();
}
