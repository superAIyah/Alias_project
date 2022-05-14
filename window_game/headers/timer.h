#ifndef TIMER_H
#define TIMER_H

#include <QTimer>
#include "itimer.h"

class Timer : public ITimer
{
public:
    Timer(int start_pos=60, int delta=-1);
    bool on();
    bool off();
    void start() override;
    void stop() override;
    void iteration();

    QTimer *timer;
    int time;
    int period = 1000; // время через которое нужно посылать сигнал
private:
    bool turned_on = false;
    int delta;
};

#endif // TIMER_H
