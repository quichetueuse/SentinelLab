cmake_minimum_required(VERSION 3.15)
project(noeud CXX)

# Forcer le standard C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Ajouter l'exécutable (remplacez main.cpp par le nom de votre fichier source)
add_executable(noeud main.cpp)

# Activer les avertissements -Wall et -Wextra (compatibles MSVC/cl)
if(MSVC)
    target_compile_options(noeud PRIVATE /W4)
else()
    target_compile_options(noeud PRIVATE -Wall -Wextra)
endif()