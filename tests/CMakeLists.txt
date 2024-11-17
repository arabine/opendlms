# Version minimale de CMake
cmake_minimum_required(VERSION 3.10)

# Nom du projet
project(cosemtest LANGUAGES C CXX)

add_subdirectory(../cosemlib csm)

# Ajouter les fichiers sources
add_executable(${PROJECT_NAME}

    main.cpp
    test_association.cpp
    cosem_tests_hal.c
)

enable_testing()

# External libraries
target_link_libraries(${PROJECT_NAME} PUBLIC cosemlib)

