cmake_minimum_required(VERSION 4.2)
project(wifi_scanner)

set(CMAKE_CXX_STANDARD 14)

include_directories(.)
include_directories(src)

add_executable(wifi_scanner
        src/about_app.cpp
        src/about_app.h
        src/app.h
        src/app_manager.cpp
        src/app_manager.h
        src/display_manager.cpp
        src/display_manager.h
        src/input_manager.cpp
        src/input_manager.h
        src/menu_app.cpp
        src/menu_app.h
        src/menu_types.h
        src/scan_app.cpp
        src/scan_app.h
        config.h
        wifi_scanner.ino)
