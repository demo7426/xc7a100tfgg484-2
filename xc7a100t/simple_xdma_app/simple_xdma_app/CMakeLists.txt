cmake_minimum_required(VERSION 3.16)
project(simple_xdma_app LANGUAGES CXX)

include(qt.cmake)

set(CMAKE_AUTORCC ON)               # 关键！Qt5必须手动打开资源编译
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

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/qcustomplot ${CMAKE_CURRENT_SOURCE_DIR}/pcie ${CMAKE_CURRENT_SOURCE_DIR}/user_test)

set(PROJECT_SOURCES
    main.cpp
    simple_xdma_app.ui
    simple_xdma_app.h
    simple_xdma_app.cpp
    simple_xdma_app.qrc
)

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} PROJECT_SOURCES)  
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/qcustomplot PROJECT_SOURCES)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/pcie PROJECT_SOURCES)

link_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/user_test 
    ${CMAKE_CURRENT_SOURCE_DIR} 
)

link_libraries(
    user_test
)

qt_add_executable(${PROJECT_NAME} ${PROJECT_SOURCES})


# 仅该目标可见，不传递给依赖它的目标
target_compile_definitions(${PROJECT_NAME} PRIVATE
    SINGLE_MODE
    VERSION="1.0"
)

set_target_properties(${PROJECT_NAME}
    PROPERTIES
        WIN32_EXECUTABLE TRUE
)

target_link_libraries(${PROJECT_NAME}
    PUBLIC
        Qt5::Core
        Qt5::Gui
        Qt5::Widgets
        Qt5::PrintSupport
)
target_precompile_headers(${PROJECT_NAME} PUBLIC stdafx.h)
