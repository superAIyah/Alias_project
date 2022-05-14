cmake_minimum_required(VERSION 3.21)
project(Alias_DB)

set(CMAKE_CXX_STANDARD 20)

add_executable(main main.cpp)

set(FULL_PATH_TO_MYSQL_CONNECTOR_CPP_DIR /home/ivan/Downloads/mysql-connector-c++-8.0.28-linux-glibc2.12-x86-64bit)

find_package(Boost COMPONENTS locale REQUIRED)
INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})

include_directories(${FULL_PATH_TO_MYSQL_CONNECTOR_CPP_DIR}/include)
include_directories(${FULL_PATH_TO_MYSQL_CONNECTOR_CPP_DIR}/include/mysqlx)
target_link_directories(main PUBLIC ${FULL_PATH_TO_MYSQL_CONNECTOR_CPP_DIR}/lib64)

add_compile_options(-lpthread)
add_definitions(-D_GLIBCXX_USE_CXX11_ABI=0)

target_link_libraries(main mysqlcppconn8)
target_link_libraries(main ${Boost_LIBRARIES})

add_library(DBConnection DBConnection.cpp)
add_library(UserDBManager UserDBManager.cpp)
add_library(WordDBManager WordDBManager.cpp)

target_link_libraries(main UserDBManager WordDBManager DBConnection)
