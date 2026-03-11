cmake_minimum_required(VERSION 3.15)

# Nom du projet
project(NimAI LANGUAGES C)

# Options générales
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# Répertoires d'includes (au cas où vous ajouteriez des .h dans includes/)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/includes)

# Fichier source principal
set(SOURCES
    src/Nim.c
)

# Création de l'exécutable
add_executable(nim_ai ${SOURCES})

# Répertoire de sortie pour l'exécutable (dans bin/)
set_target_properties(nim_ai
    PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin
)
