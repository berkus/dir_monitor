#
# Header-only library
#
cmake_minimum_required(VERSION 2.8)

include(cmake/setup.cmake)

include_directories(include)

if (BUILD_TESTING)
    add_subdirectory(tests)
endif (BUILD_TESTING)
