#include "gamewindow.h"
#include "ui_gamewindow.h"

GameWindow::GameWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GameWindow)
{
    ui->setupUi(this);
    timeController = new Timer(ui->labelTime); // создание таймера
    board = new Board(ui->tableBoard); // создание таблички
    connect(timeController->timer, SIGNAL(timeout()), this, SLOT(TimerSlot()));


}

GameWindow::~GameWindow()
{
    delete ui;
}

void GameWindow::TimerSlot()
{

    // ** Имитация таймера **
    timeController->iteration(); // итерация таймера
    Leader a("petya", 200, true);
    Leader b("fedya", 100, false);
    std::vector<Leader> lrs({a, b});
    LeaderBoard lb(lrs);
    board->UpdateLeaderboard(lb);
    /*QString prefix = "Время: ";
    QString output_time = prefix + QString::number(timeController->time);

    // ** Имитация мессенджера **
    ui->labelTime->setText(output_time);
    if (timeController->time % 2 == 0)
        ui->textBrowser->append("You: <span style='color: #3c4045'>hello</span>");
    else
        ui->textBrowser->append("Me: <span style='color: #2e73c9'>hello</span>");

    // ** Имитация таблицы **
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
