cmake_minimum_required(VERSION 3.14)
project(ls LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenCV REQUIRED)
find_package(OpenSSL REQUIRED)

include_directories(${OpenCV_INCLUDE_DIRS})
include_directories(${OPENSSL_INCLUDE_DIR})
include_directories(project)

file(GLOB SOURCES "src/*.cpp")

add_executable(ls main.cpp ${SOURCES})

target_link_libraries(ls PRIVATE ${OpenCV_LIBS} OpenSSL::SSL)