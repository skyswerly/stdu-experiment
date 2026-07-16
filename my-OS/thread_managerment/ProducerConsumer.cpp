#include "ProducerConsumer.h"

// 全局变量定义
int cnt = 5;  // 共享变量：初始产品数量
mutex mtx;    // 定义互斥锁
condition_variable cv;  // 条件变量

random_device rd;  // 全局随机数生成器
mt19937 gen(192);  // 固定种子192，保证可重复
uniform_int_distribution<> producer_dist(1000, 2000);  // 生产者耗时：1000-2000ms
uniform_int_distribution<> consumer_dist(800, 3000);   // 消费者耗时：800-3000ms

// 生产者函数实现
void Producer(int id)  // 添加id参数
{
    while (true)
    {
        // 加锁保护共享变量cnt
        unique_lock<mutex> lock(mtx);

        // 等待直至缓冲池不满时自动解锁
        cv.wait(lock, [] { return cnt < MAX; });

        // 操作共享变量
        cnt++;
        cout << "生产者[P" << id << "] 生产了一个产品！当前数量：" << cnt << endl;

        // 通知消费者
        cv.notify_one();

        // 手动解锁
        lock.unlock();

        // 模拟生产时耗时
        int sleep_time = producer_dist(gen);
        sleep_for(milliseconds(sleep_time));
    }
}

// 消费者函数实现
void Consumer(int id)  // 添加id参数
{
    while (true)
    {
        // 加锁保护共享变量cnt
        unique_lock<mutex> lock(mtx);

        // 等待直至缓冲池不空时自动解锁
        cv.wait(lock, [] { return cnt > 0; });

        // 操作共享变量
        cnt--;
        cout << "消费者[C" << id << "] 消费了一个产品！当前数量：" << cnt << endl;

        // 通知生产者
        cv.notify_one();

        // 手动解锁
        lock.unlock();

        // 模拟消费耗时
        int sleep_time = consumer_dist(gen);
        sleep_for(milliseconds(sleep_time));
    }
}

// 启动函数实现
void Start()
{
    // 创建生产者和消费者线程(3,2)
    thread producer1([](){ Producer(1); });
    thread producer2([](){ Producer(2); });
    thread producer3([](){ Producer(3); });
    thread consumer1([](){ Consumer(1); });
    thread consumer2([](){ Consumer(2); });

    // 阻塞当前线程，等待生产者和消费者完成
    producer1.join();
    producer2.join();
    producer3.join();
    consumer1.join();
    consumer2.join();
}