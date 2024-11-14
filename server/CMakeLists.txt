# Version minimale de CMake
cmake_minimum_required(VERSION 3.10)

# Nom du projet
project(cosemserver LANGUAGES C CXX)

add_subdirectory(../cosemlib csm)

# Ajouter les fichiers sources
add_library(${PROJECT_NAME}

    # Application level of cosem
    application/app_calendar.c
    application/app_database.c

    # Class IDs
    database/db_cosem_associations.c
    database/db_cosem_clock.c
    database/db_cosem_image_transfer.c

)

# Inclure des répertoires d'en-têtes si nécessaire
target_include_directories(${PROJECT_NAME} PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/application
    ${CMAKE_CURRENT_LIST_DIR}/database
)

# External libraries
target_link_libraries(${PROJECT_NAME} PUBLIC cosemlib)
