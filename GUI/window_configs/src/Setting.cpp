#include "Setting.h"

#include <utility>

Setting::Setting(QList<QRadioButton *> radios) {
    list = std::move(radios);
}

std::string Setting::collectSetting() {
    std::string s;
    for (auto & i : list) {
        if (i->isChecked())
            s += i->objectName().toStdString().substr(1);
    }
    return s;
}
