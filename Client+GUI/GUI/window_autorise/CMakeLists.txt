cmake_minimum_required(VERSION 3.5)
project(autorise_lib)

set(CMAKE_AUTOMOC ON)
set(AUTORISE_LIB_LIBRARIES)
set(AUTORISE_LIB_INCLUDE_DIRS)

find_package(Qt5 COMPONENTS Widgets REQUIRED)

file(GLOB SOURCES "*/*.*")
file(GLOB INCLUDE_DIRS "headers")

add_library(${PROJECT_NAME} STATIC ${SOURCES}) # собираем библиотеку

set(AUTORISE_LIB_LIBRARIES ${PROJECT_NAME} PARENT_SCOPE) # экспортируем библиотеку
set(AUTORISE_LIB_INCLUDE_DIRS ${INCLUDE_DIRS} PARENT_SCOPE)

target_include_directories(${PROJECT_NAME} PUBLIC ${INCLUDE_DIRS}) # для инклудов между заголовочными файлами
target_include_directories(${PROJECT_NAME} PUBLIC "../window_configs/headers") # окно авторизации вызывает окно configs
target_include_directories(${PROJECT_NAME} PUBLIC "../window_game/headers") # окно авторизации вызывает окно configs

target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::Widgets) # подключаем QT в эту библиотеку
