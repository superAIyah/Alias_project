#ifndef GAMEWINDOW_H
#define GAMEWINDOW_H

#include <QDialog>
#include "timer.h"

namespace Ui {
class GameWindow;
}

class GameWindow : public QDialog
{
    Q_OBJECT

public:
    explicit GameWindow(QWidget *parent = nullptr);
    ~GameWindow();
    Timer *timeController;

private slots:
    void on_pushButton_clicked();
    void TimerSlot();
private:
    Ui::GameWindow *ui;
};

#endif // GAMEWINDOW_H
