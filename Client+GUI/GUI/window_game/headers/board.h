#ifndef BOARD_H
#define BOARD_H

#include "IBoard.h"
#include "QTableWidget"
#include "QColor"

class Board : public IBoard
{
public:
    explicit Board(QTableWidget *table);
    void makeTable(int row, int col);
    unsigned int UpdateLeaderboard(LeaderBoard lb) override;
    void colorNick(const std::string& nick, const QColor& color);
    void colorHost(const QColor& color);
    std::string getHost();
    std::string getWinner();
private:
    QTableWidget* table; // ссылка на табличку
};

#endif // BOARD_H
