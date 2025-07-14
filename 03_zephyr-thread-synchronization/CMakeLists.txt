cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(mutex_semafor_demo)

target_sources(app PRIVATE
    src/main.c
    src/mutex_lcd.c
    src/sem_sensor.c
    src/shared_data.c
)

target_include_directories(app PRIVATE src)
