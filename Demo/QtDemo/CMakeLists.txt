cmake_minimum_required(VERSION 3.5)

project(QtDemo VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)

find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets Core REQUIRED)

find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets Core REQUIRED)

add_executable(QtDemo ExampleWidget.h ExampleWidget.cpp main.cpp)

target_link_libraries(QtDemo PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Core SpectrumEngine)

set_target_properties(QtDemo PROPERTIES 
                      FOLDER "Demo")
