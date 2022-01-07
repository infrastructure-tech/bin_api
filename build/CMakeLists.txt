
cmake_minimum_required (VERSION 3.1.1)
set (CMAKE_CXX_STANDARD 17)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY /home/eons/git/bin_cpp_api_test/./build/out)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY /home/eons/git/bin_cpp_api_test/./build/out)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY /home/eons/git/bin_cpp_api_test/./build/out)

include_directories(/home/eons/git/bin_cpp_api_test/inc)
project (entrypoint)
add_executable (entrypoint /home/eons/git/bin_cpp_api_test/src/main.cpp)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(entrypoint Threads::Threads)
target_link_libraries(entrypoint restbed cpr)
