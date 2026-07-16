#include "SJFScheduler.h"

MyThread* SJFScheduler::selectNext()
{
    MyThread* selected = nullptr;
    for (MyThread* process : readyQueue)
    {
        if (process->getState() != ProcessState::READY
            || process->getRemainTime() <= 0
            || process->getArriveTime() > currentTime)
            continue;

        if (selected == nullptr)
        {
            selected = process;
        }
        // 选总服务时间最短的；相同时选最早到达的
        else if (process->getTatalTime()
            < selected->getTatalTime())
        {
            selected = process;
        }
        else if (process->getTatalTime() == selected->getTatalTime()
                 && process->getArriveTime() < selected->getArriveTime())
        {
            selected = process;
        }
    }
    return selected;
}
