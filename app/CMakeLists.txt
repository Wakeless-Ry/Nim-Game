cmake_minimum_required(VERSION 3.16)
project(NimAI VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Dossier de sortie CORRIGÉ (pas de app/)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)

# Trouver SFML
find_package(SFML 2.5 COMPONENTS graphics window system REQUIRED)

# CHEMIN CORRIGÉ : src/nim_game.cpp (PAS app/src/)
set(SOURCE_FILE "src/nim_interface.cpp")
if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${SOURCE_FILE}")
    message(FATAL_ERROR "Fichier source manquant: ${SOURCE_FILE}")
endif()

# Créer l'exécutable
add_executable(nim_ai ${SOURCE_FILE})

# Lier SFML
target_link_libraries(nim_ai sfml-graphics sfml-window sfml-system)

# Includes
target_include_directories(nim_ai PRIVATE includes)
