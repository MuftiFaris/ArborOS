cmake_minimum_required(VERSION 3.16)
project(arbor-privacy-dashboard)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt6 COMPONENTS Core Gui Widgets DBus REQUIRED)

add_executable(arbor-privacy-dashboard
    src/main.cpp
    src/privacy-dashboard.cpp
    include/privacy-dashboard.h
)

target_link_libraries(arbor-privacy-dashboard
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::DBus
)

install(TARGETS arbor-privacy-dashboard DESTINATION bin)
