cmake_minimum_required(VERSION 3.20)
project(process_scheduling LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(BUILD_PROCESS_SCHEDULING_GUI "Build the FLTK GUI application" ON)
option(BUILD_PROCESS_SCHEDULING_CLI "Build the CLI fallback application" ON)
option(PROCESS_SCHEDULING_FETCH_FLTK "Fetch FLTK automatically with CMake FetchContent" ON)

function(enable_project_warnings target_name)
    if(MSVC)
        target_compile_options(${target_name} PRIVATE /W4)
    else()
        target_compile_options(${target_name} PRIVATE -Wall -Wextra -pedantic)
    endif()
endfunction()

set(SCHEDULING_CORE_SOURCES
    src/scheduling/SchedulingUtils.cpp
    src/scheduling/FCFSScheduler.cpp
    src/scheduling/SJFScheduler.cpp
    src/scheduling/RRScheduler.cpp
    src/scheduling/PriorityScheduler.cpp
    src/scheduling/MLQScheduler.cpp
    src/scheduling/MLFQScheduler.cpp
    src/scheduling/SchedulerFacade.cpp
)

add_library(process_scheduling_core ${SCHEDULING_CORE_SOURCES})
target_include_directories(process_scheduling_core PUBLIC src)
enable_project_warnings(process_scheduling_core)

if(BUILD_PROCESS_SCHEDULING_CLI)
    add_executable(process_scheduling_cli src/cli_main.cpp)
    target_link_libraries(process_scheduling_cli PRIVATE process_scheduling_core)
    enable_project_warnings(process_scheduling_cli)
endif()

if(BUILD_PROCESS_SCHEDULING_GUI)
    set(FLTK_TARGET_NAME "")

    if(PROCESS_SCHEDULING_FETCH_FLTK)
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
        message(FATAL_ERROR "FLTK target not found. Please enable PROCESS_SCHEDULING_FETCH_FLTK or install FLTK with CMake package config.")
    endif()

    add_executable(process_scheduling_gui
        src/main.cpp
        src/gui/MainWindow.cpp
        src/gui/GanttChartWidget.cpp
        src/gui/ResultsTable.cpp
    )
    target_include_directories(process_scheduling_gui PRIVATE src)
    if(DEFINED fltk_SOURCE_DIR AND DEFINED fltk_BINARY_DIR)
        target_include_directories(process_scheduling_gui PRIVATE ${fltk_SOURCE_DIR} ${fltk_BINARY_DIR})
    endif()
    target_link_libraries(process_scheduling_gui PRIVATE process_scheduling_core ${FLTK_TARGET_NAME})
    enable_project_warnings(process_scheduling_gui)
endif()
