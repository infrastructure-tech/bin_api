
cmake_minimum_required (VERSION 3.1.1)
set (CMAKE_CXX_STANDARD 20)
include_directories(${CMAKE_SOURCE_DIR}/../inc)

project (cpp_api_test)
add_executable (cpp_api_test ../src/main.cpp)

include_directories(${CMAKE_SOURCE_DIR}/../lib)
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_directories(cpp_api_test PUBLIC ${CMAKE_SOURCE_DIR}/../lib)
target_link_libraries(cpp_api_test Threads::Threads )
