cmake_minimum_required(VERSION 3.8)
set (CMAKE_CXX_STANDARD 20)

project(Alias_server)

find_package(Boost)
include_directories(${Boost_INCLUDE_DIRS})

file(READ config.txt FULL_PATH_TO_MYSQL_CONNECTOR_CPP_DIR)

#find_package(Boost COMPONENTS locale REQUIRED)
INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})

# Multithread Async HTTP Server
add_executable(Alias_server Server/connection.h Server/connection.cpp Server/request.h Server/async.http.server.h Server/async.http.server.cpp)
target_link_libraries(Alias_server  ${Boost_LIBRARIES} pthread)

include_directories(${FULL_PATH_TO_MYSQL_CONNECTOR_CPP_DIR}/include)
include_directories(${FULL_PATH_TO_MYSQL_CONNECTOR_CPP_DIR}/include/mysqlx)
target_link_directories(Alias_server PUBLIC ${FULL_PATH_TO_MYSQL_CONNECTOR_CPP_DIR}/lib64)

add_compile_options(-lpthread)
add_definitions(-D_GLIBCXX_USE_CXX11_ABI=0)

target_link_libraries(Alias_server mysqlcppconn8)

include_directories(DB)
add_library(DBConnection DB/DBConnection.cpp)
add_library(UserDBManager DB/UserDBManager.cpp)
add_library(WordDBManager DB/WordDBManager.cpp)

target_link_libraries(Alias_server UserDBManager WordDBManager DBConnection)

