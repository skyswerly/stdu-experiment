cmake_minimum_required(VERSION 3.16)
project(SimulatedOS LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Concurrent)

add_executable(simulated_os
    main.cpp
    main_window.h main_window.cpp

    # device — 设备管理
    device/DeviceWidget.h device/DeviceWidget.cpp
    device/banker_algorithm.h device/banker_algorithm.cpp
    device/i_banker.h

    # file — 文件管理
    file/disk_widget.h file/disk_widget.cpp
    file/disk_scheduler.h file/disk_scheduler.cpp
    file/file_widget.h file/file_widget.cpp
    file/file_system.h file/file_system.cpp
    file/algorithm_compare.h file/algorithm_compare.cpp

    # memory — 内存管理
    memory/MemoryWidget.h memory/MemoryWidget.cpp
    memory/address_generator.h memory/address_generator.cpp
    memory/i_replacer.h
    memory/fifo_replacer.h memory/fifo_replacer.cpp
    memory/lru_replacer.h memory/lru_replacer.cpp
    memory/page_table_manager.h memory/page_table_manager.cpp

    # thread — 进程管理
    thread/ThreadWidget.h thread/ThreadWidget.cpp
    thread/MyThread.h thread/MyThread.cpp
    thread/Scheduler.h thread/Scheduler.cpp
    thread/SJFScheduler.h thread/SJFScheduler.cpp
    thread/HRRNScheduler.h thread/HRRNScheduler.cpp
    thread/DynamicPriorityScheduler.h thread/DynamicPriorityScheduler.cpp
    thread/ProducerConsumer.h thread/ProducerConsumer.cpp
)

target_include_directories(simulated_os PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(simulated_os PRIVATE Qt6::Widgets Qt6::Concurrent)

# Scheduler，确保 UTF-8 编码
if(MSVC)
    target_compile_options(simulated_os PRIVATE /utf-8)
else()
    target_compile_options(simulated_os PRIVATE -fexec-charset=UTF-8)
endif()
