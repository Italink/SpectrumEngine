cmake_minimum_required(VERSION 3.16)

project(ConsoleDemo LANGUAGES CXX)

file(GLOB PROJECT_SOURCE FILES *.h *.cpp)

add_executable(ConsoleDemo
    ${PROJECT_SOURCE}
)

target_link_libraries(ConsoleDemo PRIVATE
    SpectrumEngine
)

set_target_properties(ConsoleDemo PROPERTIES 
                      FOLDER "Demo")
