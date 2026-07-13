cmake_minimum_required(VERSION 3.15)
project(thread_managerment)

set(CMAKE_CXX_STANDARD 17)

# g++ 将字符串常量编译为 UTF-8
add_compile_options(-fexec-charset=UTF-8)

# 添加可执行文件，列出所有 .cpp 源文件
add_executable(thread_sim
    main.cpp
    MyThread.cpp
    Scheduler.cpp
    SJFScheduler.cpp
    HRRNScheduler.cpp
    DynamicPriorityScheduler.cpp
    ProducerConsumer.cpp
)
