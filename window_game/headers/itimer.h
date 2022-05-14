#ifndef ITIMER_H
#define ITIMER_H


class ITimer // интерфейс таймера
{
public:
    virtual void start() = 0; //чистые виртуальные функции
    virtual void stop() = 0;
};

#endif // ITIMER_H
