project(arbor-privacy-dashboard)

cmake_minimum_required(VERSION 3.16)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 COMPONENTS
    Core
    Gui
    Widgets
    Sql
    Network
    Concurrent
    REQUIRED
)

set(SOURCES
    main.cpp
    privacy-dashboard-widget.cpp
    privacy-dashboard-widget.h
)

add_executable(arbor-privacy-dashboard ${SOURCES})

target_link_libraries(arbor-privacy-dashboard
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Sql
    Qt6::Network
    Qt6::Concurrent
)

# Install target
install(TARGETS arbor-privacy-dashboard
    RUNTIME DESTINATION bin
)
