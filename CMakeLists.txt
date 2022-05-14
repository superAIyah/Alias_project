cmake_minimum_required(VERSION 3.8)
set (CMAKE_CXX_STANDARD 20)

project(boost.asio.server)

find_package(Boost)
include_directories(${Boost_INCLUDE_DIRS})

set(FULL_PATH_TO_MYSQL_CONNECTOR_CPP_DIR /home/ivan/Downloads/mysql-connector-c++-8.0.28-linux-glibc2.12-x86-64bit)

#find_package(Boost COMPONENTS locale REQUIRED)
INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})

# Multithread Async HTTP Server
add_executable(async.http.server connection.h connection.cpp request.h response.h router.h async.http.server.h async.http.server.cpp)
target_link_libraries(async.http.server  ${Boost_LIBRARIES} pthread)

include_directories(${FULL_PATH_TO_MYSQL_CONNECTOR_CPP_DIR}/include)
include_directories(${FULL_PATH_TO_MYSQL_CONNECTOR_CPP_DIR}/include/mysqlx)
target_link_directories(async.http.server PUBLIC ${FULL_PATH_TO_MYSQL_CONNECTOR_CPP_DIR}/lib64)

add_compile_options(-lpthread)
add_definitions(-D_GLIBCXX_USE_CXX11_ABI=0)

target_link_libraries(async.http.server mysqlcppconn8)

include_directories(DB)
add_library(DBConnection DB/DBConnection.cpp)
add_library(UserDBManager DB/UserDBManager.cpp)
add_library(WordDBManager DB/WordDBManager.cpp)

target_link_libraries(async.http.server UserDBManager WordDBManager DBConnection)

