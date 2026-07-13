#ifndef SJFSCHEDULER_H
#define SJFSCHEDULER_H

#include "Scheduler.h"

// 非抢占短作业优先调度器
class SJFScheduler : public Scheduler
{
protected:
    MyThread* selectNext() override;
    const char* algorithmName() const override { return "非抢占短作业优先 (SJF) 调度模拟"; }
};

#endif // SJFSCHEDULER_H
