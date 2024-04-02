cmake_minimum_required(VERSION 3.27)
project(DES_chat)

set(CMAKE_CXX_STANDARD 17)

add_executable(DES_chat main.cpp
        src/DesOp.cpp
        src/chat.cpp)

target_include_directories(DES_chat PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
