cmake_minimum_required(VERSION 3.20)

project(kei_lang)

set(CMAKE_CXX_STANDARD 20)

# Encuentra todos los archivos fuente y de encabezado en los componentes
file(GLOB_RECURSE SOURCES
    "${CMAKE_SOURCE_DIR}/src/main.cpp"
    "${CMAKE_SOURCE_DIR}/src/Components/generator/*.cpp"
    "${CMAKE_SOURCE_DIR}/src/Components/lexing/*.cpp"
    "${CMAKE_SOURCE_DIR}/src/Components/syntax/*.cpp"
)

file(GLOB_RECURSE HEADERS
    "${CMAKE_SOURCE_DIR}/src/Components/generator/*.hpp"
    "${CMAKE_SOURCE_DIR}/src/Components/lexing/*.hpp"
    "${CMAKE_SOURCE_DIR}/src/Components/memory/*.hpp"
    "${CMAKE_SOURCE_DIR}/src/Components/syntax/*.hpp"
)

# Agregar la ejecutable principal
add_executable(kei_lang 
    ${SOURCES}
    ${HEADERS}
)

# Incluir los directorios de los componentes
target_include_directories(kei_lang PRIVATE 
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/Components/generator
    ${CMAKE_SOURCEE_DIR}/src/Components/lexing
    ${CMAKE_SOURCE_DIR}/src/Components/memory
    ${CMAKE_SOURCE_DIR}/src/Components/syntax
)

