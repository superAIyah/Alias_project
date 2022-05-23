#ifndef BOARD_H
#define BOARD_H

#include "IBoard.h"
#include "QTableWidget"

class Board : public IBoard
{
public:
    Board(QTableWidget *table);
    void makeTable(int row, int col);
    unsigned int UpdateLeaderboard(LeaderBoard lb) override;
private:
    QTableWidget* table; // ссылка на табличку
};

#endif // BOARD_H
