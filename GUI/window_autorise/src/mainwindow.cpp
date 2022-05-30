#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "User.h"

MainWindow::MainWindow(Client* cl, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow), client_(cl)
{
    ui->setupUi(this);
    configWindow = new ConfigWindow(cl);
}

//MainWindow::MainWindow(QWidget *parent)
//		: QMainWindow(parent)
//		, ui(new Ui::MainWindow)
//{
//	ui->setupUi(this);
//	configWindow = new ConfigWindow(cl);
//}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_pushButton_clicked()
{
    User user;
    user.login = ui->login->text().toStdString();
    std::string passw = ui->password->text().toStdString(); // !-- NEW: строка пороля --!
    // <-- вызов функции клиента авторизации -->
	client_->send_auth(user.login, passw);
}

void MainWindow::next_window(){
	configWindow->show();
	this->hide();
}
