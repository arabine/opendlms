# Version minimale de CMake
cmake_minimum_required(VERSION 3.10)

# Nom du projet
project(cosemlib LANGUAGES C)

# Ajouter les fichiers sources
add_library(${PROJECT_NAME} STATIC "")


target_sources(${PROJECT_NAME}

PRIVATE

    # Core cosem files
    src/csm_array.c
    src/csm_association.c
    src/csm_axdr_codec.c
    src/csm_ber.c
    src/csm_security.c
    src/csm_server.c
    src/csm_client.c
    src/csm_llc.c

    # HDLC
    hdlc/hdlc.c

    # Crypto
    crypto/aes.c
    crypto/cipher.c
    crypto/cipher_wrap.c
    crypto/gcm.c
    crypto/sha256.c
    crypto/sha1.c
    crypto/md5.c

    # Utilities
    util/bitfield.c
    util/clock.c
    util/os_util.c

PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/util/os_util.h
)


# Inclure des répertoires d'en-têtes si nécessaire
target_include_directories(${PROJECT_NAME} PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/src
    ${CMAKE_CURRENT_LIST_DIR}/util
    ${CMAKE_CURRENT_LIST_DIR}/crypto
    ${CMAKE_CURRENT_LIST_DIR}/hdlc
    ${CMAKE_CURRENT_LIST_DIR}/crypto
)


