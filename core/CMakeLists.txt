set(CORE_SOURCES
    EncryptionManager.cpp
)

set(CORE_HEADERS
    EncryptionManager.h
)

add_library(tezcrypt_core STATIC ${CORE_SOURCES} ${CORE_HEADERS})

target_include_directories(tezcrypt_core 
    PUBLIC 
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(tezcrypt_core 
    PUBLIC 
        Qt6::Core
        tezcrypt_algorithms
)
