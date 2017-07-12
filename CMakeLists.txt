cmake_minimum_required(VERSION 3.7)
project(async_downloader)

set(CMAKE_CXX_STANDARD 14)

set(SOURCE_FILES main.cpp)
add_executable(async_downloader ${SOURCE_FILES})
target_link_libraries(async_downloader curl)