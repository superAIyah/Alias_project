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
    ui(new Ui::ConfigWindow), client_(cl), gameWindow(nullptr)
{
    ui->setupUi(this);
    hideGroupBoxes({ui->BoxTeamCnt, ui->BoxTeamSz, ui->BoxLength});

	connect(this, SIGNAL(SigNextWindow()), this, SLOT(SlotNextWindow()));
}

ConfigWindow::~ConfigWindow()
{
    delete ui;
}

void ConfigWindow::MyShow(){
	if(gameWindow){delete gameWindow;}
	gameWindow = new GameWindow(client_, this);
	this->show();
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

	if (mode.collectSetting() == "1") { // если одиночный режим
//		std::cout << "One player!" << std::endl;
		team_cnt = Setting(game_mode); // то кол-во команд = 1
	}

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
	QMovie *movie = new QMovie(QString::fromStdString(gif_path));
	lbl->setMovie(movie);
	lbl->resize(200, 200);
	lbl->show();
	lbl->setAlignment(Qt::AlignCenter);
    movie->start();
    // <-- вызов функции поиска игры клиента  -->
	client_->send_settings(game_config, std::stoi(team_cnt.collectSetting()), std::stoi(round.collectSetting()));
}

void ConfigWindow::NextWindow(){
    emit SigNextWindow();
}

void ConfigWindow::hideGroupBoxes(std::vector<QGroupBox *> mas)
{
    for (auto box : mas) {
        box->setVisible(false);
    }
}

void ConfigWindow::showGroupBoxes(std::vector<QGroupBox *> mas)
{
    for (auto box : mas) {
        box->setVisible(true);
    }
}

void ConfigWindow::SlotNextWindow(){
	ui->labelGIF->hide();
	gameWindow->ShowWindow();
	this->hide();
}

void ConfigWindow::update_stats(std::string login, int win_cnt, int lose_cnt, int rating)
{
    QLabel* label_log = ui->labelLogin;
    std::string log_pref = " Логин: ";
    label_log->setText(QString::fromStdString(log_pref + login));

    QLabel* label_win = ui->labelWinCnt;
    std::string win_pref = " Кол-во побед: ";
    label_win->setText(QString::fromStdString(win_pref + std::to_string(win_cnt)));

    QLabel* label_lose = ui->labelLoseCnt;
    std::string lose_pref = " Кол-во поражений: ";
    label_lose->setText(QString::fromStdString(lose_pref + std::to_string(lose_cnt)));

    QLabel* label_rating = ui->labelRating;
    std::string rating_pref = " Общий рейтинг: ";
    label_rating->setText(QString::fromStdString(rating_pref + std::to_string(rating)));

}

void ConfigWindow::on_e1_clicked() // выбран одиночный режим
{
    hideGroupBoxes({ui->BoxTeamCnt});
    showGroupBoxes({ui->BoxTeamSz, ui->BoxLength});
    //ui->BoxTeamCnt->setVisible(false);
	ui->BoxTeamSz->setTitle("Количество игроков");
}

void ConfigWindow::on_e2_clicked() // выбран командный режим
{
    showGroupBoxes({ui->BoxTeamCnt, ui->BoxLength, ui->BoxTeamSz});
    //ui->BoxTeamCnt->setVisible(true);
	ui->BoxTeamSz->setTitle("Игроков в команде");
}

void ConfigWindow::on_pushButton_clicked()
{
    std::string rules = "Игра Alias online, Правила:\n";
    rules +=                     (std::string)"В каждом раунде есть ведущий. Ведущий получает слово и объясняет его всем остальным участникам игры.\n";
    rules +=                     (std::string)"При этом ведущему нельзя использовать само слово и все однокоренные с ним.\n";
    rules +=                     (std::string)"Цель ведущего - объяснить как можно больше слов за раунд. Цель участников - первым отгадать ";
    rules +=                     (std::string)"это слово. За каждое отгаданное слово участник получает очко, ведущий - два очка.\n";
    rules +=                     (std::string)"Ваше сообщения подсвечиваются оранжевым. Сообщение ведущего - зеленым.\n";
    rules +=                     (std::string)"Команда разработчиков \"Поднимаем мыло\" желает вам приятной игры!";
    QMessageBox::about(this, "Правила", rules.c_str());
}
