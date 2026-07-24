cmake_minimum_required(VERSION 3.24)
project(TezCrypt VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Find Qt6 with helpful error message
find_package(Qt6 COMPONENTS Core Gui Widgets REQUIRED)

if(NOT Qt6_FOUND)
    message(FATAL_ERROR 
        "Qt6 was not found. Please install Qt6 and set CMAKE_PREFIX_PATH:\n"
        "  Linux: sudo apt install qt6-base-dev qt6-declarative-dev (Ubuntu/Debian)\n"
        "  macOS: brew install qt6\n"
        "  Windows: Download from https://www.qt.io/download\n"
        "\n"
        "Or configure with:\n"
        "  cmake -DCMAKE_PREFIX_PATH=/path/to/qt6 .."
    )
endif()

# Add subdirectories for each module
add_subdirectory(algorithms)
add_subdirectory(core)
add_subdirectory(gui)
add_subdirectory(src)
