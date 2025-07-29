# CMake generated Testfile for 
# Source directory: /Users/kennethzhang/Desktop/TradingClientExchange
# Build directory: /Users/kennethzhang/Desktop/TradingClientExchange/build
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
include("/Users/kennethzhang/Desktop/TradingClientExchange/build/tce_tests[1]_include.cmake")
add_test([=[pywrapper]=] "/Users/kennethzhang/opt/miniconda3/bin/python3.9" "-m" "pytest" "-q" "/Users/kennethzhang/Desktop/TradingClientExchange/tests/test_python_wrapper.py")
set_tests_properties([=[pywrapper]=] PROPERTIES  WORKING_DIRECTORY "/Users/kennethzhang/Desktop/TradingClientExchange" _BACKTRACE_TRIPLES "/Users/kennethzhang/Desktop/TradingClientExchange/CMakeLists.txt;55;add_test;/Users/kennethzhang/Desktop/TradingClientExchange/CMakeLists.txt;0;")
