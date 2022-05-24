#include "messenger.h"

Messenger::Messenger(QTextBrowser *msg, QLabel *label)
{
    msg_browser = msg;
    label_key_word = label;
}

unsigned int Messenger::ShowMessages(const std::vector<Message>& messages)
{
    for (int i = 0; i < messages.size(); i++) {
        msg_browser->append(messages[i].msg.c_str());
    }
    return 0;
}

unsigned int Messenger::UpdateKeyword(const std::string& keyword)
{
    label_key_word->setText(keyword.c_str());
    return 0;
}

