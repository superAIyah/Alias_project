#include "timer.h"

Timer::Timer(QLabel* ui_label)
{
    timer = new QTimer();
    label = ui_label;
}

bool Timer::on()
{
    return turned_on;
}

bool Timer::off()
{
    return !turned_on;
}

void Timer::start(int start_pos)
{
    time = start_pos;
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

    QString prefix = "Время: ";
    QString output_time = prefix + QString::number(time);
    label->setText(output_time);
}
