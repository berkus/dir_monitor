#
# Header-only library
#
include(cmake/setup.cmake)

include_directories(src)

if (BUILD_TESTING)
    add_subdirectory(tests)
endif (BUILD_TESTING)
