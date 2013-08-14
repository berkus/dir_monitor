#
# This is generic boilerplate
#
set(CMAKE_CXX_FLAGS "-ferror-limit=3 ${CMAKE_CXX_FLAGS} -std=c++11 -stdlib=libc++")

# On mac, use openssl from brew, not the default system one, because it is too old.
# Run 'brew install openssl' to install it.
if (APPLE)
    set(OPENSSL_ROOT_DIR /usr/local/opt/openssl)
    # Set pkg-config path in case pkg-config is installed on the machine.
    set(ENV{PKG_CONFIG_PATH} /usr/local/opt/openssl/lib/pkgconfig)
    # A bug in cmake prevents use of OPENSSL_ROOT_DIR for finding a custom openssl,
    # so we use an internal variable instead. This needs to be fixed in cmake.
    set(_OPENSSL_ROOT_HINTS_AND_PATHS PATHS /usr/local/opt/openssl)
endif (APPLE)

find_package(OpenSSL REQUIRED)
include_directories(${OPENSSL_INCLUDE_DIR})

set(BOOST_COMPONENTS)

if (BUILD_TESTING)
    list(APPEND BOOST_COMPONENTS unit_test_framework)
    enable_testing()
    include(Dart)
endif (BUILD_TESTING)

# boost/asio depends on libboost_system
list(APPEND BOOST_COMPONENTS system)
# For logging we need boost/posix_time
list(APPEND BOOST_COMPONENTS date_time)
# Serialization is used for streaming data to and from udp packets
list(APPEND BOOST_COMPONENTS serialization)
# Program_options used to parse cmdline args in some tests
list(APPEND BOOST_COMPONENTS program_options)

#set(Boost_USE_MULTITHREAD ON)
set(Boost_USE_STATIC_LIBS ON) # Easier to deploy elsewhere
find_package(Boost REQUIRED COMPONENTS ${BOOST_COMPONENTS})

include_directories(${Boost_INCLUDE_DIR})

# Create and link a test application.
function(create_test NAME EXTRALIBS)
    add_executable(test_${NAME} test_${NAME}.cpp)
    target_link_libraries(test_${NAME} ${EXTRALIBS} ${Boost_LIBRARIES})
    #target_link_libraries(test_${NAME} supc++)
    install(TARGETS test_${NAME}
        RUNTIME DESTINATION tests/unittests)
    add_test(${NAME} test_${NAME})
endfunction(create_test)
