#ifndef BOARD_H
#define BOARD_H

#include "IBoard.h"
#include "QTableWidget"
#include "QColor"

class Board : public IBoard
{
public:
    Board(QTableWidget *table);
    void makeTable(int row, int col);
    unsigned int UpdateLeaderboard(LeaderBoard lb) override;
    void colorNick(std::string nick, QColor color);
private:
    QTableWidget* table; // ссылка на табличку
};

#endif // BOARD_H
