#ifndef MODE_H
#define MODE_H

#include <iostream>
#include "ISetting.h"
#include <QApplication>
#include <QRadioButton>

class Setting : public ISetting{
public:
    explicit Setting(QList<QRadioButton *>);
    std::string collectSetting() override;
private:
    QList<QRadioButton *> list;
};

#endif // MODE_H
