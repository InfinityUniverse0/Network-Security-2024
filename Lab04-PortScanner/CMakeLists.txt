cmake_minimum_required(VERSION 3.22)
project(PortScanner)

set(CMAKE_CXX_STANDARD 17)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

add_executable(${PROJECT_NAME} main.cpp
        src/ping.cpp
        src/tcp_connect.cpp
        src/tcp_syn.cpp
        src/tcp_fin.cpp
        src/udp.cpp)

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
