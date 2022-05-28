#include "gamewindow.h"
#include "ui_gamewindow.h"

GameWindow::GameWindow(Client* cl, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GameWindow), client_(cl)
{
    ui->setupUi(this);
    timeController = new Timer(ui->labelTime); // создание таймера
    board = new Board(ui->tableBoard); // создание таблички
    msg_browser = new Messenger(ui->textBrowser, ui->labelWord); // создание мессенджера

    gui = new ClientInterface(timeController, msg_browser, board);
    connect(timeController->timer, SIGNAL(timeout()), this, SLOT(TimerSlot()));
	connect(this, SIGNAL(Show()), this, SLOT(TimerStart()));
	connect(this, SIGNAL(Warning()), this, SLOT(SpoilerWarning()));
}

void GameWindow::ShowWindow(){
	show();
	emit Show();
}

void GameWindow::TimerStart(){
	timeController->start(client_->RoundDuration());
}

GameWindow::~GameWindow()
{
    delete ui;
}

ClientInterface *GameWindow::get_client_interface()
{
    return gui;
}

void GameWindow::TimerSlot()
{
    timeController->iteration(); // итерация таймера
//	если равен 0 то раунд
	if(timeController->time==0){
		client_->send_round();
	}
}

void GameWindow::UpdateMessages(const Message& new_msg){
	std::vector<Message> msgs({new_msg});
	gui->messenger->ShowMessages(msgs);
}

void GameWindow::UpdateLeaderboard(const LeaderBoard& lb){
	gui->board->UpdateLeaderboard(lb);
}

void GameWindow::UpdateKeyword(std::string new_kw){
	gui->messenger->UpdateKeyword(new_kw);

}

void GameWindow::SpoilerWarning() {
	QMessageBox::critical(nullptr, "Warning!", "You can't use a guessing word.");
}

void GameWindow::ShowWarning(){
	emit Warning();
}


void GameWindow::on_pushButton_clicked()
{
	std::string input_text = ui->lineEdit->text().toStdString();
	client_->send_msg(input_text);
}
