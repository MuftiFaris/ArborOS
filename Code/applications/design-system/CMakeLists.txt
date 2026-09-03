project(ArborDesignSystem)

# Design System Library - provides theme, colors, and components
add_library(arbor-design-system STATIC
    src/theme.cpp
    src/color-palette.cpp
    src/style-provider.cpp
    include/theme.h
    include/color-palette.h
    include/style-provider.h
)

target_include_directories(arbor-design-system
    PUBLIC include
)

target_link_libraries(arbor-design-system
    PUBLIC Qt6::Core Qt6::Gui Qt6::Widgets
)

# Install headers
install(DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/arbor/design-system
)

# Install stylesheets
install(FILES
    stylesheets/arbor-dark.qss
    stylesheets/arbor-light.qss
    DESTINATION ${CMAKE_INSTALL_DATADIR}/arbor/themes
)

# Install color palette JSON
install(FILES
    data/color-palette.json
    DESTINATION ${CMAKE_INSTALL_DATADIR}/arbor/design
)
