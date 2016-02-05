cmake_minimum_required(VERSION 3.2)
project(libtask)

set(CMAKE_C_FLAGS -std=gnu99)
include_directories(include)

add_subdirectory(lib)

link_libraries(task)

add_executable(httpload httpload.c)
add_executable(primes primes.c)
add_executable(tcpproxy tcpproxy.c)
add_executable(testdelay testdelay.c)

target_link_libraries(httpload)
target_link_libraries(primes)
target_link_libraries(tcpproxy)
target_link_libraries(testdelay)