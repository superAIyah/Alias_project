#ifndef MAINWINDOW_H
#define MAINWINDOW_H

class MainWindow;

#include "../../../async.http.client.h"
#include "configwindow.h"
#include <QMainWindow>
//#include "gamewindow.h"


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
	MainWindow(Client* cl, QWidget *parent = nullptr);
//	MainWindow(QWidget *parent = nullptr);

    ~MainWindow();
	void next_window();
    ConfigWindow *configWindow;
private slots:
    void on_pushButton_clicked();
private:
    Ui::MainWindow *ui;

	Client* client_;
};
#endif // MAINWINDOW_H
