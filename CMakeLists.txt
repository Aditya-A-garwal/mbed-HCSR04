
cmake_minimum_required(VERSION 3.16)

project(mbed-HCSR04
    DESCRIPTION
        "Library to use the HCSR04 ultrasonic sensor with MBed OS with asynchronous and blocking APIs"
    LANGUAGES
        CXX
)

add_library(mbed-HCSR04 INTERFACE)

target_include_directories(mbed-HCSR04
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_sources(mbed-HCSR04
    INTERFACE
        HCSR04.cpp
        HCSR04Blocking.cpp
)
