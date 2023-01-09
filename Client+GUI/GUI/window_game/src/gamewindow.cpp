#include "gamewindow.h"

#include <utility>
#include "ui_gamewindow.h"

GameWindow::GameWindow(Client *cl, ConfigWindow *prev_window, QWidget *parent) :
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
	connect(this, SIGNAL(SigShowConfig()), this, SLOT(SlotShowConfig()));

}

GameWindow::~GameWindow() {
	delete ui;
}

ClientInterface *GameWindow::get_client_interface() const {
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
	keyword = std::move(new_kw);
	emit SigUpdateKeyword();
}

void GameWindow::ShowConfig() {
	emit SigShowConfig();
}

void GameWindow::SlotTimerStart() {
	// Уведомление в чате об обновлении раунда
	std::string new_word = "<span style=' font-style:italic; text-decoration: underline;'>New Round</span>";
	Message msg(new_word, false, "");
	gui->messenger->ShowMessages({msg});

	gui->messenger->UpdateKeyword("Guess the word!");
	timeController->start(client_->RoundDuration());
}

void GameWindow::SlotSpoilerWarning() {
	QMessageBox::critical(nullptr, "SigSpoilerWarning!", "You can't use a guessing word.");
}

void GameWindow::SlotUpdateLeaderboard() { // вызывается при отгадывании слова у каждого клиента
	// напишем в чат всем пользователем, что слово обновилось
	std::string new_word = "<span style=' font-style:italic; text-decoration: underline;'>New Word</span>";
	Message msg(new_word, false, "");
	gui->messenger->ShowMessages({msg}); // показать в чат всем, что слово обновилось

	gui->board->UpdateLeaderboard(leaderboard);
	auto *board_child = (Board *) (gui->board);
	board_child->colorNick(client->getNick(), QColor(250, 168, 35)); // покраска участника
	board_child->colorHost(QColor(41, 227, 153)); // покраска хоста
}

void GameWindow::SlotUpdateMessages() {
	std::vector<Message> msgs({last_msg});

	auto *board_child = (Board *) (gui->board); // покраска сообщений хоста
	std::string color_host_pref = "<span style='color: #29e399'>";
	std::string color_host_suf = "</span>";
	std::string host_name = board_child->getHost();
	for (auto &msg: msgs) {
		Message sms = msg;
		if (sms.me) continue;
		if (sms.name == host_name)
			msg.msg = color_host_pref + sms.msg + color_host_suf;
	}

	gui->messenger->ShowMessages(msgs);
}

void GameWindow::SlotUpdateKeyword() {
	gui->messenger->UpdateKeyword(keyword);
}

void GameWindow::SlotTimerIt() {
	timeController->iteration(); // итерация таймера
//	если равен 0 то раунд
	if (timeController->time <= 0) {
		timeController->stop();
		client_->send_round();
	}
}

void GameWindow::on_pushButton_clicked() {
	std::string input_text = ui->lineEdit->text().toStdString();
	if (!input_text.empty()) {
		client_->send_msg(input_text);
		ui->lineEdit->setText("");
	}
}

void GameWindow::SlotShowConfig() {
	auto *brd = (Board *) gui->board; // чтобы вызвать дочерней метод базового класса
	std::string winner = brd->getWinner();
	QMessageBox::about(this, "Победители", winner.c_str());
	window_config->MyShow();
//    this->hide();
}
