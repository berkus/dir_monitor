#
# Header-only library

# minimum 3.1 for target_compile_features: https://cmake.org/cmake/help/v3.1/command/target_compile_features.html
# minimum 3.0 for target_include_directories: https://cmake.org/cmake/help/v3.0/command/target_include_directories.html
cmake_minimum_required(VERSION 3.1)

set_property(GLOBAL PROPERTY USE_FOLDERS ON) # Use the FOLDER target property to organize targets into folders

if (BUILD_TESTING)
    include(Dart)
endif()

# First, searching for dependencies
set (BOOST_COMPONENTS
	system
	date_time
	thread
	filesystem
)
if (BUILD_TESTING)
	list(APPEND BOOST_COMPONENTS unit_test_framework)
endif (BUILD_TESTING)
find_package(Boost COMPONENTS ${BOOST_COMPONENTS} REQUIRED)

# Then, library itself
add_library(dir_monitor INTERFACE)
target_compile_features(dir_monitor INTERFACE cxx_lambdas cxx_auto_type)
target_include_directories(dir_monitor INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)
target_include_directories(dir_monitor INTERFACE ${Boost_INCLUDE_DIRS})
target_link_libraries(dir_monitor INTERFACE ${Boost_LIBRARIES})

# Finally, install the headers and config
install(TARGETS dir_monitor EXPORT dir_monitor-config)
install(EXPORT dir_monitor-config DESTINATION lib/dir_monitor/cmake)
install(DIRECTORY include/ DESTINATION include)

# Test applications
if (BUILD_TESTING)
    add_subdirectory(tests)
endif (BUILD_TESTING)
