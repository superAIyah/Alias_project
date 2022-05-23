#ifndef ITIMER_H
#define ITIMER_H


class ITimerController // интерфейс таймера
{
public:
    //чистые виртуальные функции
    virtual void start(int) = 0; // старт таймера
    virtual void stop() = 0; // стоп таймера
    virtual void iteration() = 0; // итерация таймера на delta
    int time; // текущее время (секунды)
};

#endif // ITIMER_H
