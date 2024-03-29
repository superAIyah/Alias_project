#ifndef TIMER_H
#define TIMER_H

#include <QTimer>
#include <QLabel>
#include "iTimerController.h"

class Timer : public ITimerController
{
public:
    explicit Timer(QLabel*);
    bool on() const;
    bool off() const;
    void start(int start_pos) override;
    void stop() override;
    void iteration() override;

    QTimer *timer;
    int time{};
    int period = 1000; // время через которое нужно посылать сигнал
private:
    QLabel *label; // место куда будет отображаться время
    bool turned_on = false;
    int delta=-1;
};

#endif // TIMER_H
