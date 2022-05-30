#include "gamewindow.h"
#include "ui_gamewindow.h"

GameWindow::GameWindow(Client *cl, ConfigWindow* prev_window, QWidget *parent) :
		QDialog(parent),
		ui(new Ui::GameWindow), client_(cl) {
	ui->setupUi(this);
	timeController = new Timer(ui->labelTime); // создание таймера
	board = new Board(ui->tableBoard); // создание таблички
	msg_browser = new Messenger(ui->textBrowser, ui->labelWord); // создание мессенджера
	gui = new ClientInterface(timeController, msg_browser, board);
    window_config = prev_window;
    client = cl;

	connect(timeController->timer, SIGNAL(timeout()), this, SLOT(SlotTimerIt()));
	connect(this, SIGNAL(SigTimerStart()), this, SLOT(SlotTimerStart()));
	connect(this, SIGNAL(SigSpoilerWarning()), this, SLOT(SlotSpoilerWarning()));
	connect(this, SIGNAL(SigUpdateMessages()), this, SLOT(SlotUpdateMessages()));
	connect(this, SIGNAL(SigUpdateKeyword()), this, SLOT(SlotUpdateKeyword()));
	connect(this, SIGNAL(SigUpdateLeaderboard()), this, SLOT(SlotUpdateLeaderboard()));

}

GameWindow::~GameWindow() {
	delete ui;
}

ClientInterface *GameWindow::get_client_interface() {
	return gui;
}

void GameWindow::ShowWindow() {
	show();
	emit SigTimerStart();
}

void GameWindow::ShowWarning() {
	emit SigSpoilerWarning();
}

void GameWindow::NewRound() {
	emit SigTimerStart();
}

void GameWindow::UpdateMessages(const Message &new_msg) {
	last_msg = new_msg;
	emit SigUpdateMessages();
}

void GameWindow::UpdateLeaderboard(const LeaderBoard &lb) {
	leaderboard = lb;
	emit SigUpdateLeaderboard();
}

void GameWindow::UpdateKeyword(std::string new_kw) {
	keyword = new_kw;
	emit SigUpdateKeyword();
}

void GameWindow::SlotTimerStart() {
    gui->messenger->UpdateKeyword("Guess the word!");
	timeController->start(client_->RoundDuration());
}

void GameWindow::SlotSpoilerWarning() {
	QMessageBox::critical(nullptr, "SigSpoilerWarning!", "You can't use a guessing word.");
}

void GameWindow::SlotUpdateLeaderboard() {
	gui->board->UpdateLeaderboard(leaderboard);
    Board *board_child = (Board*)(gui->board);
    board_child->colorNick(client->getNick(), QColor(250, 168, 35));
}

void GameWindow::SlotUpdateMessages() {
	std::vector<Message> msgs({last_msg});
	gui->messenger->ShowMessages(msgs);
}

void GameWindow::SlotUpdateKeyword() {
	gui->messenger->UpdateKeyword(keyword);
}

void GameWindow::SlotTimerIt() {
	timeController->iteration(); // итерация таймера
//	если равен 0 то раунд
	if (timeController->time == 0) {
		client_->send_round();
	}
}

void GameWindow::on_pushButton_clicked() {
	std::string input_text = ui->lineEdit->text().toStdString();
	client_->send_msg(input_text);
    ui->lineEdit->setText("");
}

void GameWindow::showConfig() {
    window_config->show();
    this->hide();
}
