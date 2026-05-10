cmake_minimum_required(VERSION 3.20)
project(file_system LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(BUILD_FILE_SYSTEM_GUI "Build the FLTK GUI application" ON)
option(BUILD_FILE_SYSTEM_CLI "Build the CLI fallback application" ON)
option(FILE_SYSTEM_FETCH_FLTK "Fetch FLTK automatically with CMake FetchContent" ON)

function(enable_project_warnings target_name)
    if(MSVC)
        target_compile_options(${target_name} PRIVATE /W4)
    else()
        target_compile_options(${target_name} PRIVATE -Wall -Wextra -pedantic)
    endif()
endfunction()

set(FILE_SYSTEM_CORE_SOURCES
    src/filesystem/FsTypes.cpp
    src/filesystem/FileSystemFacade.cpp
    src/filesystem/VirtualFileSystem.cpp
)

add_library(file_system_core ${FILE_SYSTEM_CORE_SOURCES})
target_include_directories(file_system_core PUBLIC src)
enable_project_warnings(file_system_core)

if(BUILD_FILE_SYSTEM_CLI)
    add_executable(file_system_cli src/cli_main.cpp)
    target_link_libraries(file_system_cli PRIVATE file_system_core)
    enable_project_warnings(file_system_cli)
endif()

if(BUILD_FILE_SYSTEM_GUI)
    set(FLTK_TARGET_NAME "")

    if(FILE_SYSTEM_FETCH_FLTK)
        include(FetchContent)

        set(FLTK_BUILD_TEST OFF CACHE BOOL "" FORCE)
        set(FLTK_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
        set(FLTK_BUILD_FLUID OFF CACHE BOOL "" FORCE)
        set(FLTK_BUILD_FLTK_OPTIONS OFF CACHE BOOL "" FORCE)
        set(FLTK_BUILD_HTML_DOCS OFF CACHE BOOL "" FORCE)
        set(FLTK_BUILD_PDF_DOCS OFF CACHE BOOL "" FORCE)

        FetchContent_Declare(
            fltk
            GIT_REPOSITORY https://github.com/fltk/fltk.git
            GIT_TAG release-1.4.4
            GIT_SHALLOW TRUE
        )
        FetchContent_MakeAvailable(fltk)

        if(TARGET fltk::fltk)
            set(FLTK_TARGET_NAME fltk::fltk)
        elseif(TARGET fltk)
            set(FLTK_TARGET_NAME fltk)
        endif()
    else()
        find_package(FLTK REQUIRED CONFIG)
        if(TARGET fltk::fltk)
            set(FLTK_TARGET_NAME fltk::fltk)
        elseif(TARGET fltk)
            set(FLTK_TARGET_NAME fltk)
        endif()
    endif()

    if(NOT FLTK_TARGET_NAME)
        message(FATAL_ERROR "FLTK target not found. Please enable FILE_SYSTEM_FETCH_FLTK or install FLTK with CMake package config.")
    endif()

    add_executable(file_system_gui
        src/main.cpp
        src/gui/MainWindow.cpp
        src/gui/FileListTable.cpp
    )
    target_include_directories(file_system_gui PRIVATE src)
    if(DEFINED fltk_SOURCE_DIR AND DEFINED fltk_BINARY_DIR)
        target_include_directories(file_system_gui PRIVATE ${fltk_SOURCE_DIR} ${fltk_BINARY_DIR})
    endif()
    target_link_libraries(file_system_gui PRIVATE file_system_core ${FLTK_TARGET_NAME})
    enable_project_warnings(file_system_gui)
endif()
