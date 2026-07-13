#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "MyThread.h"
#include <vector>
#include <algorithm>
#include <iostream>
#include <iomanip>

using namespace std;

// 调度器抽象基类
class Scheduler
{
protected:
    vector<MyThread*> readyQueue;    // 就绪队列
    vector<MyThread*> allProcess;    // 所有进程
    int currentTime = 0;             // 当前模拟时间（ms）
    int timeSlice = 100;             // 时间片大小（ms）

    //公共工具方法

    // 快进到最早到达进程的时间
    void advanceToNextArrival();

    // 更新就绪进程的等待时间=
    void updateWaitingProcesses(int sliceStart);

    // 检查是否有已到达的就绪进程
    bool hasArrivedProcess() const;

    //虚函数

    // 选择下一个要执行的进程
    virtual MyThread* selectNext() = 0;

    // 是否为抢占式调度
    virtual bool isPreemptive() const { return false; }

    // 算法名称
    virtual const char* algorithmName() const = 0;

    // 更新进程优先级（动态优先级子类重写）
    virtual void updatePriorities() {}

public:
    virtual ~Scheduler() = default;

    // 添加进程
    void addProcess(MyThread* process);

    // 执行调度
    void schedule();

    // 显示调度结果
    void displayResults();
};

#endif // SCHEDULER_H
