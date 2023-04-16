#include "gamewindow.h"
#include "ui_gamewindow.h"

GameWindow::GameWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GameWindow)
{
    ui->setupUi(this);
    timeController = new Timer(ui->labelTime); // создание таймера
    board = new Board(ui->tableBoard); // создание таблички
    msg_browser = new Messenger(ui->textBrowser, ui->labelWord); // создание мессенджера

    gui = new ClientInterface(timeController, msg_browser, board);
    connect(timeController->timer, SIGNAL(timeout()), this, SLOT(TimerSlot()));


}

GameWindow::~GameWindow()
{
    delete ui;
}

ClientInterface *GameWindow::get_client_interface()
{
    return gui;
}

void GameWindow::TimerSlot()
{

    // ** Имитация таймера **
    timeController->iteration(); // итерация таймера

    Leader a("petya", 200, true);
    Leader b("fedya", 100, false);
    std::vector<Leader> lrs({a, b});
    LeaderBoard lb(lrs);

    gui->board->UpdateLeaderboard(lb);
    gui->messenger->UpdateKeyword("Orange");

    Message msg("Martin", 1, "DAADADADA");
    std::vector<Message> msgs({msg});
    gui->messenger->ShowMessages(msgs);
//    // ** Имитация таймера **
//    QString prefix = "Время: ";
//    QString output_time = prefix + QString::number(timeController->time);
//    ui->labelTime->setText(output_time);

    // ** Имитация мессенджера **
    //msg_browser->UpdateKeyword("Orange");
//    Message msg(0, "Martin", 1, "DAADADADA");
//    std::vector<Message> msgs({msg});
//    msg_browser->ShowMessages(msgs);
//    if (timeController->time % 2 == 0)
//        ui->textBrowser->append("You: <span style='color: #3c4045'>hello</span>");
//    else
//        ui->textBrowser->append("Me: <span style='color: #2e73c9'>hello</span>");

    /*// ** Имитация таблицы **
    QTableWidget *table = ui->tableBoard;
    table->setRowCount(5);
    table->setColumnCount(3);

    QStringList hlabels;
    hlabels << "Nick" << "Points" << "Host";
    table->setHorizontalHeaderLabels(hlabels);

    for (int i = 0; i < table->rowCount(); i++) {
        QTableWidgetItem *item;
        for (int j = 0; j < table->columnCount(); j++) {
            item = new QTableWidgetItem;
            if (j == 0)
                item->setText("Nick" + QString::number(i));
            if (j == 1)
                item->setText(QString::number(5-i));
            if (j == 2) {
                if (i == 2)
                    item->setText("*");
                else
                    item->setText("");
            }
            table->setItem(i, j, item);
        }
    }*/
}

void GameWindow::on_pushButton_clicked()
{
    timeController->start(60);
}
