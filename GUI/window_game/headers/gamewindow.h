#ifndef GAMEWINDOW_H
#define GAMEWINDOW_H

#include <QDialog>
#include <QMessageBox>
#include <string>
#include "timer.h"
#include "board.h"
#include "messenger.h"
#include "clientinterface.h"
#include "configwindow.h"

class GameWindow;

#include "../../../async.http.client.h"


namespace Ui {
	class GameWindow;
}

class GameWindow : public QDialog {
Q_OBJECT

public:
    explicit GameWindow(Client *cl, ConfigWindow*, QWidget *parent = nullptr);

	~GameWindow();

	Timer *timeController;
	Board *board;
	Messenger *msg_browser;
	ClientInterface *gui;

	ClientInterface *get_client_interface();

	void UpdateMessages(const Message &new_msg);

	void UpdateLeaderboard(const LeaderBoard &lb);

	void UpdateKeyword(std::string new_kw);

	void ShowWindow();

	void ShowWarning();

	void NewRound();


private slots:

	void on_pushButton_clicked();

	void SlotTimerIt();

	void SlotTimerStart();

	void SlotSpoilerWarning();

	void SlotUpdateLeaderboard();

	void SlotUpdateMessages();

	void SlotUpdateKeyword();

//public slots:
signals:

	void SigTimerStart();

	void SigSpoilerWarning();

	void SigUpdateLeaderboard();

	void SigUpdateMessages();

	void SigUpdateKeyword();

private:
    void showConfig();

    Client* client;
    ConfigWindow *window_config;
	Ui::GameWindow *ui;
	Client *client_;
	std::string keyword;
	LeaderBoard leaderboard;
	Message last_msg;
};

#endif // GAMEWINDOW_H
