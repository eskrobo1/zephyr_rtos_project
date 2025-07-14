cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(task_management_demo)

target_sources(app PRIVATE
    src/main.c
    src/static_dinamic_threads.c
    src/scheduling_tests.c
)
