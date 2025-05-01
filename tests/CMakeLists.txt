# Version minimale de CMake
cmake_minimum_required(VERSION 3.12)

# Nom du projet
project(cosemtest LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 20)
add_subdirectory(../cosemlib csm)

# Ajouter les fichiers sources
add_executable(${PROJECT_NAME}

    main.cpp
    test_association.cpp
    cosem_tests_hal.c
    test_hdlc.cpp
    test_clock.cpp
    test_aes128gcm.cpp
    soft_meter.cpp

    # Add Cosem class ID files
    ../server/database/db_cosem_associations.c
    ../server/database/db_cosem_clock.c

    # Add Cosem server application files
    ../server/application/app_database.c
    ../server/application/app_calendar.c
)

enable_testing()

target_include_directories(${PROJECT_NAME} PUBLIC 
    ../server/database 
    ../server/application
)

# External libraries
target_link_libraries(${PROJECT_NAME} PUBLIC cosemlib)

