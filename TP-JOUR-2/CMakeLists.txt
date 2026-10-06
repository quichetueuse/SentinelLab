cmake_minimum_required(VERSION 3.20)
project(sentinellab_jour2 CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE RelWithDebInfo)
endif()

set(AVERTISSEMENTS -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wno-sign-conversion)

# --- Nœud simulé (parties A et B) ---------------------------------------
add_executable(noeud src/noeud.cpp src/alloc_guard.cpp)
target_compile_options(noeud PRIVATE ${AVERTISSEMENTS} -fstack-usage)
target_link_libraries(noeud PRIVATE pthread)

# --- Bugs volontaires, sans et avec sanitizers (A3) ------------------------
add_executable(demo_bugs src/demo_bugs.cpp)
target_compile_options(demo_bugs PRIVATE -O0 -g)

add_executable(demo_bugs_asan src/demo_bugs.cpp)
target_compile_options(demo_bugs_asan PRIVATE -O0 -g -fsanitize=address,undefined -fno-omit-frame-pointer)
target_link_options(demo_bugs_asan PRIVATE -fsanitize=address,undefined)

# --- Tests unitaires ------------------------------------------------------
enable_testing()
add_executable(test_unitaires tests/test_unitaires.cpp)
target_compile_options(test_unitaires PRIVATE ${AVERTISSEMENTS} -fsanitize=address,undefined)
target_link_options(test_unitaires PRIVATE -fsanitize=address,undefined)
add_test(NAME unitaires COMMAND test_unitaires)
