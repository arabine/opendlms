cmake_minimum_required(VERSION 3.10)

project(opendlms)

include(ExternalProject)

ExternalProject_Add(
    tests 
    SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/tests"
    PREFIX tests
    INSTALL_COMMAND ""
)

ExternalProject_Add(
    metersimulator 
    SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/examples/metersimulator"
    PREFIX metersimulator
    INSTALL_COMMAND ""
)
