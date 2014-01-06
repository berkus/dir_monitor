include_directories(.)

create_test(sync LIBS ${Boost_LIBRARIES})
create_test(async LIBS ${Boost_LIBRARIES})
