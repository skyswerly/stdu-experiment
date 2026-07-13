#ifndef DYNAMICPRIORITYSCHEDULER_H
#define DYNAMICPRIORITYSCHEDULER_H

#include "Scheduler.h"

// 抢占式动态优先级调度器
class DynamicPriorityScheduler : public Scheduler
{
protected:
    MyThread* selectNext() override;
    bool isPreemptive() const override { return true; }
    const char* algorithmName() const override { return "动态优先级调度模拟"; }
    void updatePriorities() override;
};

#endif // DYNAMICPRIORITYSCHEDULER_H
