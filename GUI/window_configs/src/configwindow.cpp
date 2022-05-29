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
    gameWindow = new GameWindow(cl, this);
}

ConfigWindow::~ConfigWindow()
{
    delete ui;
}

void ConfigWindow::on_findGameButton_clicked()
{
	//Получаем все RadioButtons у каждого типа настройки
	QList<QRadioButton *> game_level = ui->BoxLevel->findChildren<QRadioButton *>(); // уровень сложности
	QList<QRadioButton *> game_mode = ui->BoxMode->findChildren<QRadioButton *>(); // уровень сложности
	QList<QRadioButton *> length_radios = ui->BoxLength->findChildren<QRadioButton *>(); // длина раунда
	QList<QRadioButton *> team_cnt_radios = ui->BoxTeamCnt->findChildren<QRadioButton *>(); // количество команд
	QList<QRadioButton *> team_sz_radios = ui->BoxTeamSz->findChildren<QRadioButton *>(); // размер команды

	// Определяем настройки соотв. классам
	Setting level(game_level);
	Setting mode(game_mode);
	Setting round(length_radios);
	Setting team_cnt(team_cnt_radios);
	Setting team_sz(team_sz_radios);

	std::cout << "OLD: " << team_cnt.collectSetting() << std::endl;

	if (mode.collectSetting() == "1") { // если одиночный режим
		std::cout << "One player!" << std::endl;
		team_cnt = Setting(game_mode); // то кол-во команд = 1
	}

	std::cout << "LEVEL: " << level.collectSetting() << std::endl;
	std::cout << "TEAM_SZ: " << team_sz.collectSetting() << std::endl;
	std::cout << "TEAM_CNT: " << team_cnt.collectSetting() << std::endl;
	std::cout << "ROUND: " << round.collectSetting() << std::endl;

	// Список со всеми настройками (Полиморфизм)
	std::vector<ISetting *> configs = {&level, &team_sz, &team_cnt, &round};
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
	std::cout << gif_path << std::endl;
	QMovie *movie = new QMovie(QString::fromStdString(gif_path));
	lbl->setMovie(movie);
	lbl->resize(200, 200);
	lbl->show();
	lbl->setAlignment(Qt::AlignCenter);
    movie->start();
    //movie->stop();
	//qDebug() << "abc" << "def";
//    std::string  path = QDir::currentPath().toStdString() + "/GUI/window_configs/src/loading.gif";
//    qDebug() << path.c_str();
    // <-- вызов функции поиска игры клиента  -->
	client_->send_settings(game_config, std::stoi(team_cnt.collectSetting()), std::stoi(round.collectSetting()));
}

void ConfigWindow::next_window(){
    ui->labelGIF->hide();
	gameWindow->ShowWindow();
    this->hide();
}

void ConfigWindow::on_e1_clicked() // выбран одиночный режим
{
	ui->BoxTeamCnt->setVisible(false);
	ui->BoxTeamSz->setTitle("Количество игроков");
}

void ConfigWindow::on_e2_clicked() // выбран командный режим
{
	ui->BoxTeamCnt->setVisible(true);
	ui->BoxTeamSz->setTitle("Игроков в команде");
}
