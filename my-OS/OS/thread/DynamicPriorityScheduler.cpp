#include "DynamicPriorityScheduler.h"

MyThread* DynamicPriorityScheduler::selectNext()
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
        // 选优先级最高的（数字越小优先级越高）
        else if (process->getPriority() < selected->getPriority())
        {
            selected = process;
        }
        else if (process->getPriority() == selected->getPriority()
                 && process->getArriveTime() < selected->getArriveTime())
        {
            selected = process;
        }
    }
    return selected;
}

void DynamicPriorityScheduler::updatePriorities()
{
    for (MyThread* process : readyQueue)
    {
        if (process->getState() == ProcessState::READY
            && process->getRemainTime() > 0
            && process->getArriveTime() <= currentTime)
        {
            process->updatePriority();
        }
    }
}
