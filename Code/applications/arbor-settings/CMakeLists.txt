project(ArbotSettings)

add_executable(arbor-settings
    src/main.cpp
    src/settings-window.cpp
    src/settings-model.cpp
    src/system-info.cpp
    include/settings-window.h
    include/settings-model.h
    include/system-info.h
    resources/resources.qrc
)

target_include_directories(arbor-settings
    PRIVATE include
)

target_link_libraries(arbor-settings
    PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::DBus
    Qt6::Network
    arbor-design-system
)

install(TARGETS arbor-settings
    DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(FILES
    arbor-settings.desktop
    DESTINATION ${CMAKE_INSTALL_DATADIR}/applications
)

install(FILES
    icons/arbor-settings.svg
    DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/apps
)
