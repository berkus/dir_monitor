#
# Header-only library
#
cmake_minimum_required(VERSION 2.8)
include(Dart)

# First, searching for dependencies
set (BOOST_COMPONENTS
	system
	date_time
	program_options
	thread
	filesystem)
if (BUILD_TESTING)
	list(APPEND BOOST_COMPONENTS unit_test_framework)
endif (BUILD_TESTING)
find_package(Boost COMPONENTS ${BOOST_COMPONENTS} REQUIRED)

# Then, library itself
add_library(dir_monitor INTERFACE)
target_compile_features(dir_monitor INTERFACE cxx_generic_lambdas cxx_auto_type)
target_include_directories(dir_monitor INTERFACE include)
target_include_directories(dir_monitor INTERFACE ${Boost_INCLUDE_DIRS})

# Test applications
if (BUILD_TESTING)
    add_subdirectory(tests)
endif (BUILD_TESTING)
