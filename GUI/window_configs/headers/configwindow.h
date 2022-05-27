#ifndef CONFIGWINDOW_H
#define CONFIGWINDOW_H

class ConfigWindow;

#include <QDialog>

#include "gamewindow.h"
#include "../../../async.http.client.h"

namespace Ui {
class ConfigWindow;
}

class ConfigWindow : public QDialog
{
    Q_OBJECT

public:
    explicit ConfigWindow(Client* cl, QWidget *parent = nullptr);
    ~ConfigWindow();

	void next_window();

    GameWindow *gameWindow;

private slots:
    void on_findGameButton_clicked();

private:
    Ui::ConfigWindow *ui;
	Client* client_;

};

#endif // CONFIGWINDOW_H
