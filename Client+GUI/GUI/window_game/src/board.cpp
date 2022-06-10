#include "board.h"

Board::Board(QTableWidget *ui_table) : table(ui_table)
{}

void Board::makeTable(int row, int col) {
    table->setRowCount(row); // размер таблицы
    table->setColumnCount(col);

    QStringList hlabels; // заголовки
    hlabels << "Nick" << "Points" << "Host";
    table->setHorizontalHeaderLabels(hlabels);
}

unsigned int Board::UpdateLeaderboard(LeaderBoard lb) {
//    std::cout << lb.size << std::endl;
    makeTable(lb.size, 3); // корректировка размеров таблицы
//    std::cout << "OK" << std::endl;
    //std::cout << lb.leaders[0].points;// << " " << lb.leaders[0].points << " " << lb.leaders[0].host << std::endl;
    for (int i = 0; i < lb.size; i++) {
        QTableWidgetItem *item = new QTableWidgetItem;
        item->setFlags(item->flags() ^ Qt::ItemIsEditable); // неизменяемость колонок
        Leader leader = lb.leaders[i];

        item->setText(leader.name.c_str()); // колонка имени
        table->setItem(i, 0, item);

        item = new QTableWidgetItem;
        item->setFlags(item->flags() ^ Qt::ItemIsEditable); // неизменяемость колонок
        item->setText(QString::number(leader.points)); // колонка очков
        table->setItem(i, 1, item);

        item = new QTableWidgetItem;
        item->setFlags(item->flags() ^ Qt::ItemIsEditable); // неизменяемость колонок
        if (leader.host) { // отобраежение хоста
            item->setText("*");
        }
        else {
            item->setText(" ");
        }
        table->setItem(i, 2, item);
    }
    return 0; // все OK
}

void Board::colorNick(std::string nick, QColor color)
{
    for (int i = 0; i < table->rowCount(); i++) {
        std::string nick_tmp =  table->item(i, 0)->text().toStdString();
        if (nick_tmp == nick) {
            for (int j = 0; j < table->columnCount(); j++) {
                table->item(i, j)->setBackground(color);
            }
        }
    }
}

void Board::colorHost(QColor color)
{
    for (int i = 0; i < table->rowCount(); i++) {
        std::string host =  table->item(i, 2)->text().toStdString();
        if (host == "*") {
            for (int j = 0; j < table->columnCount(); j++) {
                table->item(i, j)->setBackground(color);
            }
        }
    }
}

std::string Board::getHost()
{
    for (int i = 0; i < table->rowCount(); i++) {
        std::string host =  table->item(i, 2)->text().toStdString();
        if (host == "*") {
            return table->item(i, 0)->text().toStdString();
        }
    }
    return "";
}

std::string Board::getWinner()
{
    int max = -1;
    std::string winners = "";
    for (int i = 0; i < table->rowCount(); i++) {
        int point_tmp = table->item(i, 1)->text().toInt();
        if (point_tmp == max) { // будет несколько победителей
            winners += (std::string)"AND";
            winners += table->item(i, 0)->text().toStdString();
        }
        if (point_tmp > max) {
            winners = table->item(i, 0)->text().toStdString();
            max = point_tmp;
        }
    }
    return winners;
}
