cmake_minimum_required(VERSION 3.20)
project(memory_management LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(BUILD_MEMORY_MANAGEMENT_GUI "Build the FLTK GUI application" ON)
option(BUILD_MEMORY_MANAGEMENT_CLI "Build the CLI fallback application" ON)
option(MEMORY_MANAGEMENT_FETCH_FLTK "Fetch FLTK automatically with CMake FetchContent" ON)

function(enable_project_warnings target_name)
    if(MSVC)
        target_compile_options(${target_name} PRIVATE /W4)
    else()
        target_compile_options(${target_name} PRIVATE -Wall -Wextra -pedantic)
    endif()
endfunction()

set(MEMORY_CORE_SOURCES
    src/memory/PartitionManager.cpp
    src/memory/PageReplacement.cpp
    src/memory/MemoryFacade.cpp
)

add_library(memory_management_core ${MEMORY_CORE_SOURCES})
target_include_directories(memory_management_core PUBLIC src)
enable_project_warnings(memory_management_core)

if(BUILD_MEMORY_MANAGEMENT_CLI)
    add_executable(memory_management_cli src/cli_main.cpp)
    target_link_libraries(memory_management_cli PRIVATE memory_management_core)
    enable_project_warnings(memory_management_cli)
endif()

if(BUILD_MEMORY_MANAGEMENT_GUI)
    set(FLTK_TARGET_NAME "")

    if(MEMORY_MANAGEMENT_FETCH_FLTK)
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
        message(FATAL_ERROR "FLTK target not found. Please enable MEMORY_MANAGEMENT_FETCH_FLTK or install FLTK with CMake package config.")
    endif()

    add_executable(memory_management_gui
        src/main.cpp
        src/gui/MainWindow.cpp
        src/gui/MemoryLayoutWidget.cpp
        src/gui/BlocksTable.cpp
        src/gui/PageStepsTable.cpp
    )
    target_include_directories(memory_management_gui PRIVATE src)
    if(DEFINED fltk_SOURCE_DIR AND DEFINED fltk_BINARY_DIR)
        target_include_directories(memory_management_gui PRIVATE ${fltk_SOURCE_DIR} ${fltk_BINARY_DIR})
    endif()
    target_link_libraries(memory_management_gui PRIVATE memory_management_core ${FLTK_TARGET_NAME})
    enable_project_warnings(memory_management_gui)
endif()
