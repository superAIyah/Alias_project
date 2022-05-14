#ifndef MESSENGER_H
#define MESSENGER_H

#include <iostream>
#include <QTableWidget>
#include "ICommunication.h"

class Messenger : public ICommunication
{
public:
    Messenger();
    // Здесь находится более детализированная логика мессенджера
    QTableWidget *table; // таблица QT
};

#endif // MESSENGER_H
