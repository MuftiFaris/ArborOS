project(ArbotFiles)

add_executable(arbor-files
    src/main.cpp
    src/file-manager.cpp
    src/tag-manager.cpp
    src/preview-handler.cpp
    include/file-manager.h
    include/tag-manager.h
    include/preview-handler.h
    resources/resources.qrc
)

target_include_directories(arbor-files
    PRIVATE include
)

target_link_libraries(arbor-files
    PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Sql
    Qt6::Svg
    arbor-design-system
)

install(TARGETS arbor-files
    DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Desktop file
install(FILES
    arbor-files.desktop
    DESTINATION ${CMAKE_INSTALL_DATADIR}/applications
)

# Icon
install(FILES
    icons/arbor-files.svg
    DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/apps
)
