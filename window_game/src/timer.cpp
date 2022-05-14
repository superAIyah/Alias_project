#include "timer.h"

Timer::Timer(int start_pos /* = 60 */, int our_delta /* = -1 */ )
    : time(start_pos), delta(our_delta) {
    timer = new QTimer();
}

bool Timer::on()
{
    return turned_on;
}

bool Timer::off()
{
    return !turned_on;
}

void Timer::start()
{
    turned_on = true;
    timer->start(period);
}

void Timer::stop()
{
    turned_on = false;
    timer->stop();
}

void Timer::iteration()
{
    if (time != 0)
        time += delta;
}
