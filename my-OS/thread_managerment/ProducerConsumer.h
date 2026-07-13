#ifndef PRODUCER_CONSUMER_H
#define PRODUCER_CONSUMER_H

#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <iostream>
#include <random>

#define MAX 20  // 缓冲池最大值为20

using namespace std;
using namespace this_thread;
using namespace chrono;

// 全局变量声明
extern int cnt;
extern mutex mtx;
extern condition_variable cv;
extern random_device rd;
extern mt19937 gen;
extern uniform_int_distribution<> producer_dist;
extern uniform_int_distribution<> consumer_dist;

// 函数声明
void Producer();
void Consumer();
void Start();

#endif // PRODUCER_CONSUMER_H
