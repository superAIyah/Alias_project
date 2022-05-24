#include "Setting.h"

Setting::Setting(QList<QRadioButton *> radios) {
    list = radios;
    return;
}

std::string Setting::collectSetting() {
    std::string s = "";
    for (int i = 0; i < list.size(); i++) {
        if (list[i]->isChecked())
            s += list[i]->objectName().toStdString();
    }
    return s;
}
