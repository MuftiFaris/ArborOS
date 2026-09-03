project(ArbotSystemMonitor)

add_executable(arbor-system-monitor
    src/main.cpp
    src/monitor-window.cpp
    src/system-metrics.cpp
    include/monitor-window.h
    include/system-metrics.h
    resources/resources.qrc
)

target_include_directories(arbor-system-monitor
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../design-system/include
)

target_link_libraries(arbor-system-monitor
    PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Charts
    Qt6::Network
    arbor-design-system
)

install(TARGETS arbor-system-monitor
    DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(FILES
    arbor-system-monitor.desktop
    DESTINATION ${CMAKE_INSTALL_DATADIR}/applications
)

install(FILES
    icons/arbor-system-monitor.svg
    DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/apps
)
