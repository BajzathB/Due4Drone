cmake_minimum_required(VERSION 3.16)

project(KalmanFilter_SIL
    VERSION 1.0
    LANGUAGES CXX
)

# C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Kalman Filter library
add_library(kalman_filter
    ../test/helper/support4Testing.cpp
    ../src/Controller.cpp
    ../src/sysTime.cpp
    ../src/RC.cpp
    ../src/MotorControl.cpp
    ../src/SPI_common.cpp
    ../src/SPI.cpp
    ../src/SPI_SDcard.cpp
    ../src/LED.cpp
)

target_include_directories(kalman_filter
    PUBLIC
    ../test/helper/support4Testing.hpp
    ../test/helper/support4Testing.h
    ../src/Controller.h
    ../src/sysTime.h
    ../src/RC.h
    ../src/MotorControl.h
    ../src/SPI_common.h
    ../src/SPI.h
    ../src/SPI_SDcard.h
    ../src/LED.h
)

# SIL executable
add_executable(kalman_sil
    sil_kf.cpp
    )

add_compile_definitions(UNIT_TEST)
target_compile_options(kalman_sil PRIVATE
    -g
    -O0
)

target_link_libraries(kalman_sil
    PRIVATE kalman_filter
)

# Copy measurement data into build directory
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/results)

# Enable testing
# enable_testing()