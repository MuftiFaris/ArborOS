project(ArborApplicationsTests)

# Unit tests
add_executable(test-design-system
    test-color-palette.cpp
    test-theme.cpp
)

target_link_libraries(test-design-system
    PRIVATE
    Qt6::Core
    Qt6::Test
    arbor-design-system
)

add_test(NAME DesignSystemTests COMMAND test-design-system)
