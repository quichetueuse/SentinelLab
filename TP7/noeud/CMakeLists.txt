find_package(PkgConfig REQUIRED)
pkg_check_modules(MOSQ REQUIRED libmosquitto)

add_executable(noeud main.cpp)
target_include_directories(noeud PRIVATE ${MOSQ_INCLUDE_DIRS})
target_link_libraries(noeud PRIVATE ${MOSQ_LIBRARIES})
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -g")

# Création de l'exécutable pour le test de vulnérabilité
add_executable(parse_vuln parse_vuln.cpp)

# Activation d'ASan UNIQUEMENT pour cet exécutable
target_compile_options(parse_vuln PRIVATE -fsanitize=address -g)
target_link_options(parse_vuln PRIVATE -fsanitize=address)
