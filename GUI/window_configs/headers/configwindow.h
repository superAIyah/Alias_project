#ifndef CONFIGWINDOW_H
#define CONFIGWINDOW_H

class ConfigWindow;

#include <QDialog>

#include "gamewindow.h"
#include "../../../async.http.client.h"

class GameWindow;

namespace Ui {
class ConfigWindow;
}

class ConfigWindow : public QDialog
{
    Q_OBJECT

public:
    explicit ConfigWindow(Client* cl, QWidget *parent = nullptr);
    ~ConfigWindow();

	void MyShow();

	void NextWindow();
    void update_stats(std::string login, int win_cnt, int lose_cnt, int rating);

    GameWindow *gameWindow;

private slots:
	void SlotNextWindow();
    void on_findGameButton_clicked();
	void on_e1_clicked();
	void on_e2_clicked();
	
signals:
	void SigNextWindow();

private:
    Ui::ConfigWindow *ui;
	Client* client_;

};

#endif // CONFIGWINDOW_H
