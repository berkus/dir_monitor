function(create_test NAME)
    cmake_parse_arguments(CT "NO_CTEST" "" "LIBS" ${ARGN})
    add_executable(test_${NAME} test_${NAME}.cpp)
    target_link_libraries(test_${NAME} ${CT_LIBS} ${Boost_LIBRARIES} dir_monitor)
    target_compile_definitions(test_${NAME} PRIVATE BOOST_TEST_DYN_LINK)
    install(TARGETS test_${NAME}
        RUNTIME DESTINATION tests/unittests)
    if (NOT CT_NO_CTEST)
        add_test(${NAME} test_${NAME})
    endif (NOT CT_NO_CTEST)
endfunction(create_test)

include_directories(.)

if (APPLE)
    find_library(COREFOUNDATION_LIB CoreFoundation)
    find_library(CORESERVICES_LIB CoreServices)
endif (APPLE)

create_test(sync LIBS ${Boost_LIBRARIES} ${COREFOUNDATION_LIB} ${CORESERVICES_LIB})
create_test(async LIBS ${Boost_LIBRARIES} ${COREFOUNDATION_LIB} ${CORESERVICES_LIB})
create_test(running LIBS ${Boost_LIBRARIES} ${COREFOUNDATION_LIB} ${CORESERVICES_LIB})
