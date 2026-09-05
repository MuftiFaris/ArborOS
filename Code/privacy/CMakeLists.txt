cmake_minimum_required(VERSION 3.16)

project(privacy_framework VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt5 COMPONENTS Core Gui REQUIRED)
find_package(SQLite3 REQUIRED)

# Add include directory
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# Privacy Framework Library
add_library(privacy_framework STATIC
    include/privacy-manager.h
    include/permission-policy.h
    src/privacy-manager.cpp
    src/permission-policy.cpp
)

target_link_libraries(privacy_framework
    Qt5::Core
    Qt5::Gui
    SQLite::SQLite3
)

# Create executable for testing (optional)
add_executable(privacy_test
    src/privacy-manager.cpp
    src/permission-policy.cpp
    include/privacy-manager.h
    include/permission-policy.h
    test/main.cpp
)

target_link_libraries(privacy_test
    Qt5::Core
    Qt5::Gui
    SQLite::SQLite3
)

# Installation
install(TARGETS privacy_framework
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

install(FILES
    include/privacy-manager.h
    include/permission-policy.h
    DESTINATION include/arbor/privacy
)
