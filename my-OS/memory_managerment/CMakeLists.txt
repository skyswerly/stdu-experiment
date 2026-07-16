cmake_minimum_required(VERSION 3.16)
project(memory_managerment)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Qt6 自动预处理（moc/uic/rcc）
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

add_executable(memory_managerment
    main.cpp
    main_window.h
    main_window.cpp
    address_generator.h
    address_generator.cpp
    i_replacer.h
    fifo_replacer.h
    fifo_replacer.cpp
    lru_replacer.h
    lru_replacer.cpp
    page_table_manager.h
    page_table_manager.cpp
)

target_link_libraries(memory_managerment PRIVATE Qt6::Widgets)
