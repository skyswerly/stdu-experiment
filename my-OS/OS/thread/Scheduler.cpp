#include "Scheduler.h"
#include <climits>

// 添加进程
void Scheduler::addProcess(MyThread* process)
{
    allProcess.push_back(process);
    readyQueue.push_back(process);
}

// 检查是否有已到达的就绪进程
bool Scheduler::hasArrivedProcess() const
{
    for (MyThread* process : readyQueue)
    {
        if (process->getState() == ProcessState::READY
            && process->getRemainTime() > 0
            && process->getArriveTime() <= currentTime)
        {
            return true;
        }
    }
    return false;
}

// 快进到最早到达时间
void Scheduler::advanceToNextArrival()
{
    int minArrival = INT_MAX;
    for (MyThread* process : readyQueue)
    {
        if (process->getState() == ProcessState::READY
            && process->getRemainTime() > 0
            && process->getArriveTime() < minArrival)
        {
            minArrival = process->getArriveTime();
        }
    }
    if (minArrival != INT_MAX && minArrival > currentTime)
    {
        std::cout << "⏩ 时间推进到 " << minArrival << "ms（等待进程到达）" << std::endl;
        currentTime = minArrival;
    }
}

// 更新就绪进程的等待时间
void Scheduler::updateWaitingProcesses(int sliceStart)
{
    for (MyThread* process : readyQueue)
    {
        // 只更新当前时间片开始前已到达的就绪进程
        if (process->getState() == ProcessState::READY
            && process->getRemainTime() > 0
            && process->getArriveTime() <= sliceStart)
        {
            process->updateWaitTime(timeSlice);
        }
    }
}

// 执行调度
void Scheduler::schedule()
{
    std::cout << "===== " << algorithmName() << " =====" << std::endl;
    std::cout << "时间片: " << timeSlice << "ms" << std::endl << std::endl;

    // 按到达时间排序
    std::sort(readyQueue.begin(), readyQueue.end(), [](MyThread* a, MyThread* b)
    {
        return a->getArriveTime() < b->getArriveTime();
    });

    int totalProcesses = readyQueue.size();
    int finishProcesses = 0;

    while (finishProcesses < totalProcesses)
    {
        //1. 检查是否有已到达进程
        if (!hasArrivedProcess())
        {
            advanceToNextArrival();
            continue;
        }

        //2. 更新优先级
        updatePriorities();

        //3. 选择下一个进程
        MyThread* selected = selectNext();

        std::cout << "----------------------------------------" << std::endl;
        std::cout << "时间 " << currentTime << "ms: 选择进程 "
             << selected->getName() << " (优先级: " << selected->getPriority() << ")" << std::endl;

        if (isPreemptive())
        {
            //抢占式：执行一个时间片
            selected->run(currentTime);
            selected->join();

            currentTime += timeSlice;

            // 更新等待进程
            updateWaitingProcesses(currentTime - timeSlice);
            updatePriorities();

            // 检查是否完成
            if (selected->getRemainTime() <= 0)
            {
                selected->finish(currentTime);
                finishProcesses++;
                std::cout << "✅ 进程 " << selected->getName() << " 已完成！" << std::endl;
                readyQueue.erase(
                    std::remove(readyQueue.begin(), readyQueue.end(), selected),
                    readyQueue.end());
            }
            else
            {
                selected->suspend();
            }
        }
        else
        {
            //非抢占式：运行到完成
            do
            {
                selected->run(currentTime);
                selected->join();

                currentTime += timeSlice;

                int sliceStart = currentTime - timeSlice;
                updateWaitingProcesses(sliceStart);
                updatePriorities();

            } while (selected->getRemainTime() > 0);

            selected->finish(currentTime);
            finishProcesses++;
            std::cout << "✅ 进程 " << selected->getName() << " 已完成！" << std::endl;
            readyQueue.erase(
                std::remove(readyQueue.begin(), readyQueue.end(), selected),
                readyQueue.end());
        }
    }

    std::cout << "========================================" << std::endl;
    std::cout << "所有进程调度完成！" << std::endl;
}

// 显示调度结果
void Scheduler::displayResults()
{
    std::cout << "\n========== 调度结果汇总 ==========" << std::endl;

    double totalTurnaround = 0;
    double totalWeighted = 0;
    int count = 0;

    for (auto* process : allProcess)
    {
        process->displayInfo();

        int turnaround = process->calculateTurnaroundTime();
        if (turnaround != -1)
        {
            totalTurnaround += turnaround;
            totalWeighted += process->calculateWeightedTurnaroundTime();
            count++;
        }
    }

    if (count > 0)
    {
        std::cout << "\n========== 平均指标 ==========" << std::endl;
        std::cout << "平均周转时间: " << std::fixed << std::setprecision(2)
             << totalTurnaround / count << "ms" << std::endl;
        std::cout << "平均带权周转时间: " << std::fixed << std::setprecision(2)
             << totalWeighted / count << std::endl;
    }
}
