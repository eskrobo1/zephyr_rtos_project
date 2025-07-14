cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(task_communication_demo)

target_sources(app PRIVATE
    src/main.c
    src/queue_demo.c
    src/pipe_demo.c
)
