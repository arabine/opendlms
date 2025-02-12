cmake_minimum_required(VERSION 3.10)

# Nom du projet
project(libmetersim LANGUAGES C CXX)

add_subdirectory(../../../server csmsrv)

# Ajouter les fichiers sources
add_executable(${PROJECT_NAME}

    libmetersim.c
    ../src/cosem_server_hal.c
)

# Inclure des répertoires d'en-têtes si nécessaire
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/../src
    ${CMAKE_SOURCE_DIR}/../system
)

# External libraries
target_link_libraries(${PROJECT_NAME} PUBLIC cosemserver cosemlib)

# Spécifier les options de compilation pour Emscripten
set_target_properties(${PROJECT_NAME} PROPERTIES
    COMPILE_FLAGS "-s WASM=1"
    LINK_FLAGS "-s WASM=1 -s EXPORTED_FUNCTIONS='[\"_libmetersim_initialize\"]' -s EXPORTED_RUNTIME_METHODS='[\"ccall\", \"cwrap\"]'"
)
