cmake_minimum_required(VERSION 3.27)
project(EncChat)

set(CMAKE_CXX_STANDARD 17)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

add_executable(EncChat main.cpp
        src/DesOp.cpp
        src/chat.cpp
        src/RSA.cpp)

target_include_directories(EncChat PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

if(WIN32)
    target_link_libraries(EncChat ws2_32)
endif()
