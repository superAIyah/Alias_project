cmake_minimum_required(VERSION 3.5)

project(GUI_lib LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt5 COMPONENTS Widgets REQUIRED)

add_subdirectory(window_autorise) # собрать окно авторизации
add_subdirectory(window_configs) # собрать окно конфигураций
add_subdirectory(window_game)

message("AUTORISE_LIB_INCLUDE_DIRS = ${AUTORISE_LIB_INCLUDE_DIRS}") # проверка сборки окна авторизации
message("AUTORISE_LIB_LIBRARIES = ${AUTORISE_LIB_LIBRARIES}")
message("CONFIGS_INCLUDE_DIRS = ${CONFIGS_INCLUDE_DIRS}") # проверка сборки окна авторизации
message("CONFIGS_LIB_LIBRARIES = ${CONFIGS_LIB_LIBRARIES}")
message("GAME_INCLUDE_DIRS = ${GAME_INCLUDE_DIRS}") # проверка сборки окна игры
message("GAME_LIB_LIBRARIES = ${GAME_LIB_LIBRARIES}")


add_library(GUI_lib STATIC) # создаем библиотеку всего GUI

target_link_libraries(${PROJECT_NAME} PUBLIC Qt5::Widgets)
target_include_directories(${PROJECT_NAME} PUBLIC ${AUTORISE_LIB_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PUBLIC ${AUTORISE_LIB_LIBRARIES}) # подключение библиотека окна авторизации
target_include_directories(${PROJECT_NAME} PUBLIC ${CONFIGS_LIB_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PUBLIC ${CONFIGS_LIB_LIBRARIES}) # подключение библиотека окна конфигураций
target_include_directories(${PROJECT_NAME} PUBLIC ${GAME_LIB_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PUBLIC ${GAME_LIB_LIBRARIES}) # подключение библиотека окна игры