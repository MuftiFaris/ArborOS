project(ArbotUpdateManager)

add_executable(arbor-update-manager
    src/main.cpp
    src/update-window.cpp
    src/update-checker.cpp
    include/update-window.h
    include/update-checker.h
    resources/resources.qrc
)

target_include_directories(arbor-update-manager
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../design-system/include
)

target_link_libraries(arbor-update-manager
    PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::DBus
    Qt6::Network
    arbor-design-system
)

install(TARGETS arbor-update-manager
    DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(FILES
    arbor-update-manager.desktop
    DESTINATION ${CMAKE_INSTALL_DATADIR}/applications
)

install(FILES
    icons/arbor-update-manager.svg
    DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/apps
)
