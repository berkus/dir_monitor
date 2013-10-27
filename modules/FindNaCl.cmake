# FindNaCl.cmake
if (NOT NACL_FOUND)
    find_path(NACL_INCLUDE_DIRS
        NAMES crypto_box_curve25519xsalsa20poly1305.h
        PATHS
            ${NACL_PREFIX}/include
            /usr/include
            /usr/include/nacl
            /usr/local/include
            /opt/local/include
            /usr/local/opt/nacl/include/
        NO_DEFAULT_PATH
    )
    find_library(NACL_LIBRARIES
        NAMES libnacl.a
        PATHS
            ${NACL_PREFIX}/lib
            /usr/lib
            /usr/lib/nacl
            /usr/local/lib
            /usr/local/lib/nacl
            /opt/local/lib
            /usr/local/opt/nacl/lib/
        NO_DEFAULT_PATH
    )
    if (NACL_INCLUDE_DIRS AND NACL_LIBRARIES)
        message(STATUS "libnacl found")
        set(NACL_FOUND TRUE)
    endif()
endif()
