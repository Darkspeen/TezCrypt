set(GUI_SOURCES
    MainWindow.cpp
)

set(GUI_HEADERS
    MainWindow.h
)

add_library(tezcrypt_gui STATIC ${GUI_SOURCES} ${GUI_HEADERS})

target_include_directories(tezcrypt_gui 
    PUBLIC 
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(tezcrypt_gui 
    PUBLIC 
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        tezcrypt_core
        tezcrypt_algorithms
)
