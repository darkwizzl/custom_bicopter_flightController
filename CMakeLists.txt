cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

set(PICO_BOARD pico_w)
 
include(pico_sdk_import.cmake)

project(project1 C CXX ASM)

pico_sdk_init()
    
add_executable(project1 
                main.cpp
                ibus/ibus.cpp
                imu/mpu6050.cpp
                imu/hmc5883l.cpp)

target_link_libraries(project1 
                      pico_stdlib
                      hardware_uart
                      hardware_i2c
                      hardware_pwm)

pico_enable_stdio_usb(project1 1)
pico_enable_stdio_uart(project1 0)

target_include_directories(project1 PRIVATE 
                            ${CMAKE_CURRENT_LIST_DIR}
                            ${CMAKE_CURRENT_LIST_DIR}/ibus
                            ${CMAKE_CURRENT_LIST_DIR}/imu)




pico_add_extra_outputs(project1)