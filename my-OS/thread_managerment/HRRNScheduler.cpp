#include "HRRNScheduler.h"

MyThread* HRRNScheduler::selectNext()
{
    MyThread* selected = nullptr;
    double maxRatio = -1.0;

    for (MyThread* process : readyQueue)
    {
        if (process->getState() != ProcessState::READY
            || process->getRemainTime() <= 0
            || process->getArriveTime() > currentTime)
            continue;

        // 响应比 R = (等待时间 + 服务时间) / 服务时间
        double ratio = static_cast<double>(process->getWaitTime() + process->getTatalTime())
                       / process->getTatalTime();

        if (ratio > maxRatio)
        {
            maxRatio = ratio;
            selected = process;
        }
        else if (ratio == maxRatio && selected
                 && process->getArriveTime() < selected->getArriveTime())
        {
            // 响应比相同时选最早到达
            selected = process;
        }
    }
    return selected;
}
