set(ALGORITHMS_SOURCES
    CaesarCipher.cpp
    ROT13.cpp
)

set(ALGORITHMS_HEADERS
    EncryptionAlgorithm.h
    CaesarCipher.h
    ROT13.h
)

add_library(tezcrypt_algorithms STATIC ${ALGORITHMS_SOURCES} ${ALGORITHMS_HEADERS})

target_include_directories(tezcrypt_algorithms 
    PUBLIC 
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(tezcrypt_algorithms 
    PUBLIC 
        Qt6::Core
)
