cmake_minimum_required(VERSION 3.16)

project(GraphAlgorithms
    VERSION 1.0
    LANGUAGES CXX
)

# Standard C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Optymalizacja (ważne przy testowaniu algorytmów)
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS_RELEASE "-O3")

# Pliki źródłowe
file(GLOB_RECURSE SOURCES
    src/*.cpp
)

# Tworzenie programu
add_executable(graph_algorithms ${SOURCES})

# Folder z nagłówkami
target_include_directories(graph_algorithms PRIVATE include)

# Statyczne linkowanie (ważne dla Windows)
if(MINGW)
    target_link_options(graph_algorithms PRIVATE
        -static
        -static-libgcc
        -static-libstdc++
    )
endif()

# Ostrzeżenia kompilatora
if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(graph_algorithms PRIVATE
        -Wall
        -Wextra
        -Wpedantic
    )
endif()

# Informacja diagnostyczna
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER}")