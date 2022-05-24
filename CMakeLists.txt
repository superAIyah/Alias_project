cmake_minimum_required(VERSION 3.18.4)
project(Alias)

set(CMAKE_CXX_STANDARD 11)

add_subdirectory(GUI)
add_executable(Alias async.http.client.cpp request.h response.h)
