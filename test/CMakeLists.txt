cmake_minimum_required(VERSION 3.16)

project(Due4Drone_UT LANGUAGES CXX)

# Use C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Enable testing
enable_testing()

# Find GoogleTest
find_package(GTest REQUIRED)

# add_compile_definitions(UNIT_TEST)

# List of unit test files
set(TEST_FILES
    test_systime.cpp
    test_RC.cpp
    test_motorControl.cpp
    test_SPI.cpp
    test_Controller.cpp
    test_input2output.cpp
    test_BT.cpp
)

# Build everything into ONE executable
add_executable(unit_tests
    ${TEST_FILES}
    helper/support4Testing.h
    helper/support4Testing.hpp
    helper/support4Testing.cpp
    ../src/sysTime.cpp
    ../src/sysTime.h
    ../src/RC.cpp
    ../src/RC.h
    ../src/LED.cpp
    ../src/LED.h
    ../src/MotorControl.cpp
    ../src/MotorControl.h
    ../src/SPI.cpp
    ../src/SPI.h
    ../src/SPI_common.cpp
    ../src/SPI_common.h
    ../src/SPI_SDcard.cpp
    ../src/SPI_SDcard.h
    ../src/Controller.cpp
    ../src/Controller.h
    ../src/BT.cpp
    ../src/BT.h
    ../src/BT_IDs.h
)

target_compile_definitions(unit_tests PRIVATE UNIT_TEST)

target_compile_options(unit_tests PRIVATE
    -g
    -O0
)

# Link GoogleTest
target_link_libraries(unit_tests
    PRIVATE
    GTest::gtest_main
)

# Register the executable with CTest
include(GoogleTest)
gtest_discover_tests(unit_tests)