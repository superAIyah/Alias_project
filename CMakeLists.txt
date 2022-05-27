cmake_minimum_required(VERSION 3.18.4)
project(Alias)

set(CMAKE_CXX_STANDARD 11)

find_package(Boost)
include_directories(${Boost_INCLUDE_DIRS})

add_compile_options(-lpthread)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
#add_definitions(-D_GLIBCXX_USE_CXX11_ABI=0)

add_subdirectory(GUI)
add_executable(Alias async.http.client.cpp request.h response.h)
target_link_libraries(Alias PUBLIC GUI_lib)
