cmake_minimum_required(VERSION 3.16)
project(simple_xdma_app LANGUAGES CXX)

include(qt.cmake)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR}
    COMPONENTS
        Core
        Gui
        Widgets
        PrintSupport
)
qt_standard_project_setup()


list(APPEND CMAKE_AUTOUIC_SEARCH_PATHS ${CMAKE_CURRENT_SOURCE_DIR})

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../qcustomplot ${CMAKE_CURRENT_SOURCE_DIR}/../pcie)

set(PROJECT_SOURCES
    main.cpp
    simple_xdma_app.ui
    simple_xdma_app.h
    simple_xdma_app.cpp
)

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} PROJECT_SOURCES)  
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/../qcustomplot PROJECT_SOURCES)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/../pcie PROJECT_SOURCES)

qt_add_executable(${PROJECT_NAME} ${PROJECT_SOURCES})

set_target_properties(${PROJECT_NAME}
    PROPERTIES
        WIN32_EXECUTABLE TRUE
)

target_link_libraries(${PROJECT_NAME}
    PUBLIC
        Qt::Core
        Qt::Gui
        Qt::Widgets
        Qt::PrintSupport
)
target_precompile_headers(${PROJECT_NAME} PUBLIC stdafx.h)
