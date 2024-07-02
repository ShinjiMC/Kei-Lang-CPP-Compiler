cmake_minimum_required(VERSION 3.20)

project(kei_lang)

set(CMAKE_CXX_STANDARD 20)

add_executable(kei_lang src/main.cpp)

target_include_directories(kei_lang PRIVATE 
    ${CMAKE_SOURCE_DIR}/src  # Asume que todos los archivos .hpp están en una carpeta 'include'
)