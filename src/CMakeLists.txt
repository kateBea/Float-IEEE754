cmake_minimum_required(VERSION 3.2)

set(MY_PROJECT_NAME "IEE754_Converter")
set(EXEC_NAME "iee754")
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(${MY_PROJECT_NAME})

add_executable(${EXEC_NAME} main.cc)
