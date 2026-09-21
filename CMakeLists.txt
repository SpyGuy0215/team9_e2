cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(my_app)

target_sources(app PRIVATE
    src/main.c
    src/system_zero.c
    src/system_one.c
    src/parser.c
)
