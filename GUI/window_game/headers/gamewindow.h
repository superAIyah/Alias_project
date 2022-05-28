#ifndef GAMEWINDOW_H
#define GAMEWINDOW_H

#include <QDialog>
#include <QMessageBox>
#include "timer.h"
#include "board.h"
#include "messenger.h"
#include "clientinterface.h"

class GameWindow;

#include "../../../async.http.client.h"


namespace Ui {
class GameWindow;
}

class GameWindow : public QDialog
{
    Q_OBJECT

public:
    explicit GameWindow(Client* cl, QWidget *parent = nullptr);
	void CreateTimer();
    ~GameWindow();
    Timer *timeController;
    Board *board;
    Messenger *msg_browser;
    ClientInterface* gui;
    ClientInterface* get_client_interface();
	void UpdateMessages(const Message& new_msg);
	void UpdateLeaderboard(const LeaderBoard& lb);
	void UpdateKeyword(std::string new_kw);
	void spoiler_warning();
	void ShowWindow();


private slots:
    void on_pushButton_clicked();
    void TimerSlot();
	void TimerStart();

public slots:
signals:
	void Show();
private:
    Ui::GameWindow *ui;
	Client* client_;
};

#endif // GAMEWINDOW_H
