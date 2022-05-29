#include "configwindow.h"
#include "ui_configwindow.h"
#include "Setting.h"
#include "gamebuilder.h"

#include <iostream>
#include <QDebug>
#include <QDir>
#include <QMovie>
#include <QMessageBox>

ConfigWindow::ConfigWindow(Client* cl, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ConfigWindow), client_(cl)
{
    ui->setupUi(this);
    gameWindow = new GameWindow(cl);
}

ConfigWindow::~ConfigWindow()
{
    delete ui;
}

void ConfigWindow::on_findGameButton_clicked()
{
    //Получаем все RadioButtons у каждого типа настройки
	QList<QRadioButton *> game_mode = ui->BoxMode->findChildren<QRadioButton *>(); // уровень сложности
	QList<QRadioButton *> length_radios = ui->BoxLength->findChildren<QRadioButton *>(); // длина раунда
	QList<QRadioButton *> team_cnt_radios = ui->BoxTeamCnt->findChildren<QRadioButton *>(); // количество команд
	QList<QRadioButton *> team_sz_radios = ui->BoxTeamSz->findChildren<QRadioButton *>(); // размер команды

	// Определяем настройки соотв. классам
	Setting mode = Setting(game_mode);
	Setting round = Setting(length_radios);
	Setting team_cnt = Setting(team_cnt_radios);
	Setting team_sz = Setting(team_sz_radios);

	// Список со всеми настройками (Полиморфизм)
	std::vector<ISetting *> configs = {&mode, &team_sz, &team_cnt, &round};
    int settings_cnt = configs.size();

    // Создаем класс, собирающий все настройки воедино
    GameBuilder configs_builder(configs);
    GameConfig game_config;
    game_config = configs_builder.CollectSettings();
    qDebug() << game_config.GetSettings().c_str();
    if (game_config.size != settings_cnt) {
        QMessageBox::critical(this, "Ошибка", "Выберите все настройки");
        return;
    }

    QLabel *lbl = ui->labelGIF; // запуск гифки поиска игры
    std::string gif_path = QDir::currentPath().toStdString() + "/GUI/window_configs/src/loading.gif";
    QMovie *movie = new QMovie(QString::fromStdString(gif_path));
    lbl->setMovie(movie);
    lbl->resize(200, 200);
    lbl->show();
    lbl->setAlignment(Qt::AlignCenter);
    movie->start();
    //qDebug() << "abc" << "def";
//    std::string  path = QDir::currentPath().toStdString() + "/GUI/window_configs/src/loading.gif";
//    qDebug() << path.c_str();
    // <-- вызов функции поиска игры клиента  -->
	client_->send_settings(game_config, std::stoi(team_cnt.collectSetting()), std::stoi(round.collectSetting()));
}

void ConfigWindow::next_window(){
	gameWindow->ShowWindow();
    this->hide();
}
