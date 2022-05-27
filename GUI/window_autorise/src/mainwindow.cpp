#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "User.h"

MainWindow::MainWindow(Client* cl, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow), client_(cl)
{
    ui->setupUi(this);
    configWindow = new ConfigWindow();
}

MainWindow::MainWindow(QWidget *parent)
		: QMainWindow(parent)
		, ui(new Ui::MainWindow)
{
	ui->setupUi(this);
	configWindow = new ConfigWindow();
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_pushButton_clicked()
{
    User user;
    user.login = ui->login->text().toStdString();
    // <-- вызов функции клиента авторизации -->
	client_->send_auth(user.login);

//    if (user.login == "test") {
//        ui->statusbar->showMessage("Вы успешно авторизовались!");
//        configWindow->show();
//        this->hide();
//    } else {
//        ui->statusbar->showMessage("Ошибка авторизации.");
//    }
//    configWindow->show();
//    this->hide();
}

void MainWindow::next_window(){
	configWindow->show();
	this->hide();
}
