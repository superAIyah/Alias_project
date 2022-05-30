#include "messenger.h"

Messenger::Messenger(QTextBrowser *msg, QLabel *label)
{
	msg_browser = msg;
	label_key_word = label;
}

unsigned int Messenger::ShowMessages(const std::vector<Message>& messages)
{
    std::string color_pref = "<span style='color: #faa823'>"; // to browse own messages
    std::string color_suf = "</span>";
	std::string between = ": ";
	for (int i = 0; i < messages.size(); i++) {
        std::string sms = messages[i].name + between + messages[i].msg;
        if (messages[i].me) {
            sms = messages[i].name + between + color_pref + messages[i].msg + color_suf;
        }
        msg_browser->append(sms.c_str());
	}
	return 0;
}

unsigned int Messenger::UpdateKeyword(const std::string& keyword)
{
	label_key_word->setText(keyword.c_str());
	return 0;
}
