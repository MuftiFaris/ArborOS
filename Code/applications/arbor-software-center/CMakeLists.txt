project(arbor-software-center LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 COMPONENTS Core Gui Widgets Network Sql REQUIRED)

add_executable(arbor-software-center
    src/main.cpp
    src/app-store.cpp
    src/flatpak-manager.cpp
    include/app-store.h
    include/flatpak-manager.h
    resources/resources.qrc
)

target_include_directories(arbor-software-center
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../design-system/include
)

target_link_libraries(arbor-software-center
    PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Network
    Qt6::Sql
    arbor-design-system
)

install(TARGETS arbor-software-center
    DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(FILES
    arbor-software-center.desktop
    DESTINATION ${CMAKE_INSTALL_DATADIR}/applications
)

install(FILES
    icons/arbor-software-center.svg
    DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/apps
)
