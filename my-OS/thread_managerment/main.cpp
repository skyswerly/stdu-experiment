#include "DynamicPriorityScheduler.h"
#include "SJFScheduler.h"
#include "HRRNScheduler.h"
#include "MyThread.h"
#include "ProducerConsumer.h"

#include <cstdlib>
#include <vector>
#include <memory>

int main()
{
    // 设置控制台编码为 UTF-8
    system("chcp 65001 > nul");

    /*
                ===== 生产者-消费者问题模拟 =====
    */
    /*
    cout << "===== 生产者-消费者问题模拟 =====" << endl;
    cout << "初始产品数量：" << cnt << endl;
    cout << "缓冲池最大容量：" << MAX << endl;
    cout << "生产者耗时范围：1000-2000ms" << endl;
    cout << "消费者耗时范围：800-3000ms" << endl;
    cout << "==================================" << endl << endl;

    // 启动生产者消费者模拟
    Start();
    */

    /*
                ===== 进程调度算法模拟（多态） =====
    */

    // 1. 选择调度算法
    int choice = 0;
    cout << "请选择调度算法：" << endl;
    cout << "1. 非抢占短作业优先算法 (SJF)" << endl;
    cout << "2. 最高响应比优先算法 (HRRN)" << endl;
    cout << "3. 动态优先级调度算法" << endl;
    cout << "请输入选项 (1-3): ";
    cin >> choice;

    // 2. 用父类指针创建调度器（多态）
    Scheduler* scheduler = nullptr;
    if (choice == 1)
    {
        scheduler = new SJFScheduler();
    }
    else if (choice == 2)
    {
        scheduler = new HRRNScheduler();
    }
    else if (choice == 3)
    {
        scheduler = new DynamicPriorityScheduler();
    }
    else
    {
        cout << "非法选项！" << endl;
        return 1;
    }

    // 3. 输入进程并添加到调度器中
    int count = 0;
    cout << "请输入待调度进程个数: ";
    cin >> count;

    // 存储进程对象（调度器只存指针，对象生命周期由本 vector 管理）
    // 用指针避免 vector 重分配时触发拷贝（std::thread 不可拷贝）
    vector<MyThread*> processes;

    for (int i = 1; i <= count; ++i)
    {
        cout << "\n--- 第 " << i << " 个进程 ---" << endl;

        int pid;
        string name;
        int totalTime;
        int basePriority;
        int arriveTime;

        cout << "PID: ";
        cin >> pid;
        cout << "进程名称: ";
        cin >> name;
        cout << "所需 CPU 时间 (ms): ";
        cin >> totalTime;
        cout << "基础优先级 (1-20, 数字越小优先级越高): ";
        cin >> basePriority;
        cout << "到达时间 (ms): ";
        cin >> arriveTime;

        MyThread* p = new MyThread(pid, name, totalTime, basePriority, arriveTime);
        processes.push_back(p);
        scheduler->addProcess(p);
    }

    // 4. 执行调度
    cout << endl;
    scheduler->schedule();

    // 5. 显示调度结果
    scheduler->displayResults();

    // 6. 清理
    for (auto* p : processes)
    {
        delete p;
    }
    delete scheduler;
    return 0;
}
