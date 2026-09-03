cmake_minimum_required(VERSION 3.24)
project(ArborOS-Applications VERSION 0.6.0 LANGUAGES CXX)

# C++ Standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Find Qt6
find_package(Qt6 REQUIRED COMPONENTS
    Core
    Gui
    Widgets
    Sql
    DBus
    Network
    Concurrent
    Svg
)

# Compiler flags
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    add_compile_options(-Wall -Wextra -Wpedantic -fPIC)
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        add_compile_options(-O3 -march=native)
    endif()
endif()

# Arbor Design System Library
add_subdirectory(design-system)

# Core Applications
add_subdirectory(arbor-files)
add_subdirectory(arbor-terminal)
add_subdirectory(arbor-settings)
add_subdirectory(arbor-software-center)
add_subdirectory(arbor-update-manager)
add_subdirectory(arbor-system-monitor)

# Installation
install(DIRECTORY
    DESTINATION ${CMAKE_INSTALL_DATADIR}/arbor/applications
)

# Testing
enable_testing()
add_subdirectory(tests)

# Summary
message(STATUS "ArborOS Applications Build Configuration")
message(STATUS "========================================")
message(STATUS "Version: ${PROJECT_VERSION}")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Qt6 Version: ${Qt6_VERSION}")
message(STATUS "========================================")
