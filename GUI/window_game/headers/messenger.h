#ifndef MESSENGER_H
#define MESSENGER_H

#include <iostream>
#include <QTextBrowser>
#include <QLabel>
#include "IMessenger.h"

class Messenger : public IMessenger
{
public:
    Messenger(QTextBrowser*, QLabel*); // куда выводить сообщения/отгадываемое слово
    unsigned int ShowMessages(const std::vector<Message>& messages) override;
    unsigned int UpdateKeyword(const std::string& keyword) override;
private:
    QTextBrowser *msg_browser; // таблица QT
    QLabel *label_key_word;
};

#endif // MESSENGER_H
