set(APP_SOURCES
    main.cpp
)

add_executable(tezcrypt ${APP_SOURCES})

target_link_libraries(tezcrypt 
    PRIVATE 
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        tezcrypt_gui
        tezcrypt_core
        tezcrypt_algorithms
)

# Set output directory for the executable
set_target_properties(tezcrypt PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
