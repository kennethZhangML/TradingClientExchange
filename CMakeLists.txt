cmake_minimum_required(VERSION 3.20)
project(TradingClientExchange LANGUAGES CXX)

# ────────── compiler flags ───────────────────────────────────────────────
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS  OFF)
if (MSVC)
    add_compile_options(/W4 /permissive-)
else()
    add_compile_options(-Wall -Wextra -pedantic)
endif()

set(PROJECT_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include)
set(API_PUBLIC_DIR      ${PROJECT_SOURCE_DIR}/src)         # for api_c.h

add_library(tce_core
    src/Order.cpp
    src/OrderBook.cpp
    src/ExecutionEngine.cpp
    src/EngineRunner.cpp
    src/api_c.cpp               
    src/utils/Logger.cpp)
target_include_directories(tce_core PUBLIC
    ${PROJECT_INCLUDE_DIR}
    ${API_PUBLIC_DIR})

# ────────── demo executable ─────────────────────────────────────────────
add_executable(TradingClientExchange src/main.cpp)
target_link_libraries(TradingClientExchange PRIVATE tce_core)

# ────────── Google‑Test suite ───────────────────────────────────────────
option(BUILD_TESTS "Build unit tests" ON)
if (BUILD_TESTS)
    enable_testing()
    find_package(GTest REQUIRED)

    add_executable(tce_tests
        tests/OrderTests.cpp
        tests/OrderBookTests.cpp
        tests/ExecutionEngineTests.cpp
        tests/EngineRunnerTests.cpp)
    target_link_libraries(tce_tests PRIVATE tce_core
                                          GTest::gtest
                                          GTest::gtest_main)

    include(GoogleTest)
    gtest_discover_tests(tce_tests)
endif()

find_package(Python3 COMPONENTS Interpreter REQUIRED)

add_test(
    NAME pywrapper              # ctest -R pywrapper
    COMMAND ${Python3_EXECUTABLE} -m pytest -q
            ${CMAKE_SOURCE_DIR}/tests/test_python_wrapper.py
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)
