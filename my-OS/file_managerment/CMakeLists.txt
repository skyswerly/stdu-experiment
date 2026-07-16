cmake_minimum_required(VERSION 3.16)
project(file_managerment)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

# Qt6 路径
set(CMAKE_PREFIX_PATH "E:/IDE/Qt/6.11.1/mingw_64")
find_package(Qt6 REQUIRED COMPONENTS Widgets)

add_executable(file_managerment
    main.cpp
    main_window.h main_window.cpp
    disk_widget.h disk_widget.cpp
    disk_scheduler.h disk_scheduler.cpp
    file_widget.h file_widget.cpp
    file_system.h file_system.cpp
    algorithm_compare.h algorithm_compare.cpp
)

target_link_libraries(file_managerment PRIVATE Qt6::Widgets)
