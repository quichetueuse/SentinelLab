find_package(PkgConfig REQUIRED)
pkg_check_modules(MOSQ REQUIRED libmosquitto)

add_executable(noeud main.cpp)
target_include_directories(noeud PRIVATE ${MOSQ_INCLUDE_DIRS})
target_link_libraries(noeud PRIVATE ${MOSQ_LIBRARIES})