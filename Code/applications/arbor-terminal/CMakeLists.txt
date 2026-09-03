project(ArbotTerminal)

add_executable(arbor-terminal
    src/main.cpp
    src/terminal-widget.cpp
    src/terminal-window.cpp
    include/terminal-widget.h
    include/terminal-window.h
    resources/resources.qrc
)

target_include_directories(arbor-terminal
    PRIVATE include
)

target_link_libraries(arbor-terminal
    PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    arbor-design-system
)

install(TARGETS arbor-terminal
    DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(FILES
    arbor-terminal.desktop
    DESTINATION ${CMAKE_INSTALL_DATADIR}/applications
)

install(FILES
    icons/arbor-terminal.svg
    DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/apps
)
