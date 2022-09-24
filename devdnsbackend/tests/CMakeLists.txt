set(TEST_EXEC "test_devdnsbackend")
ADD_EXECUTABLE(${TEST_EXEC} main.cpp)
target_link_libraries(${TEST_EXEC} devdnsbackend)
target_include_directories(${TEST_EXEC} PRIVATE ../include)
target_include_directories(${TEST_EXEC} PRIVATE ../src)
target_include_directories(${TEST_EXEC} PRIVATE ${pdns_SOURCE_DIR})
ADD_TEST(NAME ${TEST_EXEC} COMMAND ${TEST_EXEC})

