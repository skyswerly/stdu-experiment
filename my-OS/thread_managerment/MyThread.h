#ifndef MY_THREAD_H
#define MY_THREAD_H

#include<thread>
#include<chrono>
#include<string>
#include<atomic>
#include<iostream>

using namespace std;
using namespace chrono;

//进程状态枚举
enum class ProcessState
{
    READY,      //就绪
    RUNNING,    //运行
    FINISH,     //终止
};

class MyThread
{
private:
    thread work;//工作进程

    int pid;//进程id
    string name;//进程名称

    ProcessState state;//当前状态
    bool isRunning;//是否正在运行

    int totalTime;//总CPU时间
    int remainTime;//剩余CPU时间

    int arriveTime;//到达时间
    int startTime;//开始时间
    int finishTime;//完成时间
    int waitTime;//等待时间

    int priority;//当前优先级
    int basePriority;//基础优先级

public:

    //部分参数构造
    MyThread(int pid,const string& name,int totalTime,int basePriority,int arriveTime);

    //析构函数
    ~MyThread();

    //======GET方法======
    int getPID() const
    {
        return pid;
    }
    string getName() const
    {
        return name;
    }
    ProcessState getState() const
    {
        return state;
    }
    int getPriority() const
    {
        return priority;
    }
    int getTatalTime() const
    {
        return totalTime;
    }
    int getRemainTime() const
    {
        return remainTime;
    }
    int getArriveTime() const
    {
        return arriveTime;
    }
    int getWaitTime() const
    {
        return waitTime;
    }
    int getStartTime()const
    {
        return startTime;
    }
    int getFinishTime()const
    {
        return finishTime;
    }

    //======SET方法======
    void setPriority(int priority)
    {
        this->priority = priority;
    }
    void setState(ProcessState state)
    {
        this->state = state;
    }

    //进程调度相关
    void run(int currentTime);
    void suspend();
    void finish(int currentTime);

    //更新指标
    void updateWaitTime(int time);
    void updateRemainTime(int time);
    void updatePriority();

    //计算调度指标
    int calculateTurnaroundTime() const;//计算周转时间
    double calculateWeightedTurnaroundTime() const; // 计算带权周转时间

    //线程控制
    void start();
    void join();
    void detach();

    //信息打印
    void displayInfo()const;

private:
    void execute();
};

#endif // MY_THREAD_H
