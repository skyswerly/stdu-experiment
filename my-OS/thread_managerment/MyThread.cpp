#include"MyThread.h"

#include <iomanip>

using namespace chrono;


//部分参数构造
MyThread::MyThread(int pid,const string& name,int totalTime,int basePriority,int arriveTime):pid(pid),name(name),totalTime(totalTime),basePriority(basePriority),arriveTime(arriveTime){
    this->remainTime = totalTime;
    this->startTime = -1;//未开始
    this->finishTime = -1;//未完成
    this->waitTime = 0;
    this->priority = basePriority;//初始优先级 = 基础优先级
    this->state = ProcessState::READY;
    this->isRunning = false;
}

//析构函数
MyThread::~MyThread(){
    if(work.joinable()){
        work.detach();
    }
}

//线程运行
void MyThread::run(int currentTime){
    //1.修改状态标志
    if(state == ProcessState::READY){
        state = ProcessState::RUNNING;
        isRunning = true;
    }

    //2.记录开始时间
    if(startTime == -1){
        startTime = currentTime;
    }

    //3.确保之前的线程已结束再创建新线程
    if(work.joinable()){
        work.join();
    }
    work = thread(&MyThread::execute,this);
}

//等待线程完成当前时间片
void MyThread::join(){
    if(work.joinable()){
        work.join();
    }
}

//线程挂起
void MyThread::suspend()
{
    //修改状态标志
    if(state == ProcessState::RUNNING){
        state = ProcessState::READY;
        isRunning = false;
    }
}

//线程结束
void MyThread::finish(int currentTime)
{
    //1.修改状态标志
    state = ProcessState::FINISH;
    isRunning = false;

    //2.记录完成时间
    if(finishTime == -1){
        finishTime = currentTime;
    }

    //3.安全清理线程
    if (work.joinable())
    {
        work.join();
    }
}

//执行函数：只执行一个时间片（100ms 或剩余时间取最小值）
void MyThread::execute()
{
    cout<<"["<<name<<"]开始执行(PID:"<<pid<<")"<<endl;

    //模拟执行一个时间片
    const int TIME_SLICE = 100;
    int executeTime = min(TIME_SLICE,remainTime);

    //模拟CPU执行
    this_thread::sleep_for(milliseconds(executeTime));

    //更新CPU剩余时间
    updateRemainTime(executeTime);

    cout<<"["<<name<<"]执行了"<<executeTime<<"ms,剩余："<<remainTime<<"ms"<<endl;

    //时间片结束，标记运行结束
    isRunning = false;
}

//更新等待时间
void MyThread::updateWaitTime(int time)
{
    if (state == ProcessState::READY)
    {
        waitTime += time;
    }
}

//更新剩余时间
void MyThread::updateRemainTime(int time)
{
    remainTime = max(0,remainTime - time);
}

//动态更新优先级（范围：1-20，数字越小优先级越高）
void MyThread::updatePriority()
{
    //动态优先级公式
    int newPriority = basePriority - waitTime / 100;
    priority = max(1,min(20,newPriority));
}

//计算周转时间
int MyThread::calculateTurnaroundTime() const
{
    //如果到达时间和结束时间均有意义才可计算
    if (finishTime != -1 && arriveTime != -1)
    {
        return finishTime - arriveTime;
    }

    return -1;
}

//计算带权周转时间
double MyThread::calculateWeightedTurnaroundTime() const
{
    //带权周转时间 = 周转时间 / 服务时间
    if (finishTime != -1 && arriveTime != -1 && totalTime > 0)
    {
        return static_cast<double>(calculateTurnaroundTime()) / totalTime;
    }

    return -1;
}


void MyThread::displayInfo() const
{
    cout << "========================================" << endl;
    cout << "进程: " << name << " (PID: " << pid << ")" << endl;
    cout << "  状态: ";
    switch(state) {
    case ProcessState::READY: cout << "就绪"; break;
    case ProcessState::RUNNING: cout << "运行"; break;
    case ProcessState::FINISH: cout << "终止"; break;
    }
    cout << endl;
    cout << "  服务时间: " << totalTime << "ms" << endl;
    cout << "  剩余时间: " << remainTime << "ms" << endl;
    cout << "  到达时间: " << arriveTime << "ms" << endl;
    cout << "  开始时间: " << (startTime != -1 ? to_string(startTime) : "未开始") << endl;
    cout << "  完成时间: " << (finishTime != -1 ? to_string(finishTime) : "未完成") << endl;
    cout << "  等待时间: " << waitTime << "ms" << endl;

    int turnaround = calculateTurnaroundTime();
    if (turnaround != -1) {
        cout << "  周转时间: " << turnaround << "ms" << endl;
        double weighted = calculateWeightedTurnaroundTime();
        cout << "  带权周转时间: " << fixed << setprecision(2) << weighted << endl;
    } else {
        cout << "  周转时间: 未完成" << endl;
    }
    cout << "========================================" << endl;
}
