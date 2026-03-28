cmake_minimum_required(VERSION 3.16)
project(NimAI VERSION 1.0 LANGUAGES C CXX)

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Includes
include_directories(includes)

# Sources (.c + .cpp)
set(SOURCES
    src/main.cpp
    src/Game.c
    src/Interface.cpp
    src/DrawingUtils.cpp
    src/AI.c
    src/menu.cpp
    includes/menu.hpp
    includes/Game.h
    includes/AI.h
    includes/Interface.hpp
    includes/DrawingUtils.hpp
)

# Exécutable
add_executable(nim_ai ${SOURCES})

# SFML Ubuntu 2.6.1 → BIBLIOTHÈQUES SYSTÈME
find_package(SFML 2.5 COMPONENTS graphics window system REQUIRED)
target_link_libraries(nim_ai PRIVATE 
    sfml-graphics      # ← Noms Ubuntu (pas SFML::Graphics)
    sfml-window
    sfml-system
)

# Math pour sin() dans Interface.cpp
target_link_libraries(nim_ai PRIVATE m)

# Cible finale C++
set_target_properties(nim_ai PROPERTIES LINKER_LANGUAGE CXX)
