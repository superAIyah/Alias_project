cmake_minimum_required(VERSION 3.16)
project(AliasProject)

find_package(Qt5 COMPONENTS Widgets)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Boost)
include_directories(${Boost_INCLUDE_DIRS})

add_compile_options(-lpthread)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
#add_definitions(-D_GLIBCXX_USE_CXX11_ABI=0)

add_subdirectory(GUI)
add_subdirectory(Client)
