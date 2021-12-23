cmake_minimum_required(VERSION 3.16)

project(SpectrumEngine LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

file(GLOB PROJECT_SOURCE FILES Src/*.h Src/*.cpp)

add_subdirectory(3rdParty/fftw3)
add_subdirectory(Demo)

add_library(SpectrumEngine
    ${PROJECT_SOURCE}
)
set_target_properties(fftw3 PROPERTIES 
                      FOLDER "3rdParty")

target_link_libraries(SpectrumEngine PUBLIC fftw3)

target_include_directories(
    SpectrumEngine PUBLIC 
    Src
    3rdparty/fftw3/api
)

