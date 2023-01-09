#ifndef CONFIGWINDOW_H
#define CONFIGWINDOW_H

class ConfigWindow;

#include <QDialog>
#include <QGroupBox>

#include "gamewindow.h"
#include "../../../Client/async.http.client.h"

class GameWindow;

namespace Ui {
class ConfigWindow;
}

class ConfigWindow : public QDialog
{
    Q_OBJECT

public:
    explicit ConfigWindow(Client* cl, QWidget *parent = nullptr);
    ~ConfigWindow() override;

	void MyShow();

	void NextWindow();
    static void hideGroupBoxes(const std::vector<QGroupBox*>&);
    static void showGroupBoxes(const std::vector<QGroupBox*>&);
    void update_stats(const std::string& login, unsigned int win_cnt, unsigned int lose_cnt, unsigned int rating);

    GameWindow *gameWindow;

private slots:
	void SlotNextWindow();
    void on_findGameButton_clicked();
	void on_e1_clicked();
	void on_e2_clicked();
	
    void on_pushButton_clicked();

signals:
	void SigNextWindow();

private:
    Ui::ConfigWindow *ui;
	Client* client_;

};

#endif // CONFIGWINDOW_H
