#ifndef ISETTING_H
#define ISETTING_H
#include <iostream>

class ISetting{ // интферейс настроек
    // каждая настройка должна уметь возвращать выбранные пользователем параметры
public:
    virtual std::string collectSetting() = 0;
};

#endif // ISETTING_H
