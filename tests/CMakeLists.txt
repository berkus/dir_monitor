include_directories(.)

if (APPLE)
    find_library(COREFOUNDATION_LIB CoreFoundation)
    find_library(CORESERVICES_LIB CoreServices)
endif (APPLE)

create_test(sync LIBS ${Boost_LIBRARIES} ${COREFOUNDATION_LIB} ${CORESERVICES_LIB})
create_test(async LIBS ${Boost_LIBRARIES} ${COREFOUNDATION_LIB} ${CORESERVICES_LIB})
create_test(running LIBS ${Boost_LIBRARIES} ${COREFOUNDATION_LIB} ${CORESERVICES_LIB})
