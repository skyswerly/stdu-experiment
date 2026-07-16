#ifndef HRRNSCHEDULER_H
#define HRRNSCHEDULER_H

#include "Scheduler.h"

// 非抢占高响应比优先调度器
class HRRNScheduler : public Scheduler
{
protected:
    MyThread* selectNext() override;
    const char* algorithmName() const override { return "高响应比优先 (HRRN) 调度模拟"; }
};

#endif // HRRNSCHEDULER_H
