cmake_minimum_required(VERSION 3.16)

project(adept-tree VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt5 REQUIRED COMPONENTS Core Sql Test Widgets)

enable_testing()

add_subdirectory(src)
add_subdirectory(tests)
