include(FetchContent)
include(ExternalProject)

cmake_minimum_required(VERSION 3.8)
set(CMAKE_CXX_STANDARD 17)

set(LIB_NAME "devdnsbackend")
set(PDNS_TAG "auth-4.6.3")
set(UACME_TAG "upstream/1.7.3")
set(DEPENDENCIES)
set(PQXX_TAG "7.7.4")

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

project(${LIB_NAME})

# fmt dep
find_library(FMT_LIBRARIES fmt REQUIRED)

# uacme
find_library(UACME_LIBRARIES uacme REQUIRED)

# pq
find_library(PQ_LIBRARY pq REQUIRED)
find_package(gpgsqlbackend REQUIRED)
find_path(PQ_INCLUDE_DIR postgresql/libpq-fe.h)

# pdns headers
FetchContent_Declare(
        pdns
        GIT_REPOSITORY https://github.com/PowerDNS/pdns.git
        GIT_TAG ${PDNS_TAG}
)
FetchContent_MakeAvailable(pdns)
FetchContent_GetProperties(pdns)

# uacme
FetchContent_Declare(
        uacme
        GIT_REPOSITORY https://github.com/ndilieto/uacme.git
        GIT_TAG ${UACME_TAG}
)

FetchContent_MakeAvailable(uacme)
FetchContent_GetProperties(uacme)

IF (NOT EXISTS ${uacme_SOURCE_DIR}/config.h)
    exec_program(
        ./configure ${uacme_SOURCE_DIR}
        ARGS --disable-maintainer-mode --disable-docs --no-recursion --with-openssl
    )
    IF (${uacme_SOURCE_DIR}/config.h IS_NEWER_THAN ${CMAKE_CURRENT_SOURCE_DIR}/include/uacme/config.h )
        file(COPY ${uacme_SOURCE_DIR}/config.h DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/include/uacme/)
    ENDIF ()
ENDIF ()
# TODO add check to dependency libcurl gnutls ...

# acme_lw
FetchContent_Declare(
        acme_lw
        GIT_REPOSITORY https://git.loopback.it/andrea/acme-lw.git
        GIT_TAG master
)

FetchContent_MakeAvailable(acme_lw)
FetchContent_GetProperties(acme_lw)



add_library(${LIB_NAME} MODULE src/${LIB_NAME}.cpp)
target_include_directories(${LIB_NAME} PRIVATE include)
target_include_directories(${LIB_NAME} PRIVATE src)
# fmt
target_link_libraries(${LIB_NAME} PRIVATE ${FMT_LIBRARIES})
# uacme
target_link_libraries(${LIB_NAME} PRIVATE ${UACME_LIBRARIES})
# pq
target_link_libraries(${LIB_NAME} PRIVATE ${PQ_LIBRARY} ${GPSQLBACKEND_LIBRARY})
target_include_directories(${LIB_NAME} PRIVATE ${PQ_INCLUDE_DIR}/postgresql)
# pdns headers
target_include_directories(${LIB_NAME} PRIVATE ${pdns_SOURCE_DIR})
# acme_lw
target_include_directories(${LIB_NAME} PRIVATE ${acme_lw_SOURCE_DIR})
target_link_libraries(${LIB_NAME} PRIVATE acme_lw)
# pqxx
find_package(PQXX)
if (NOT PQXX_FOUND)
    message("Building pqxx from source ...")
    include(FetchContent)
    FetchContent_Declare(
            libpqxx
            GIT_REPOSITORY https://github.com/jtv/libpqxx.git
            GIT_TAG ${PQXX_TAG}
    )
    set(BUILD_SHARED_LIBS ON)
    set(BUILD_DOC OFF)
    set(SKIP_BUILD_TEST ON)
    set(PQXX_LIBRARIES pqxx)

    FetchContent_MakeAvailable(libpqxx)
endif ()
target_link_libraries(${LIB_NAME} PRIVATE ${PQXX_LIBRARIES})

# install
install(TARGETS ${LIB_NAME} DESTINATION lib/pdns)

# tests
project(tests)
enable_testing()

function(add_test_binary test_name source_file)
    add_executable(${test_name} ${source_file})
    target_include_directories(${test_name} PRIVATE ${pdns_SOURCE_DIR})
    target_link_libraries(${test_name} PRIVATE ${UACME_LIBRARIES})
    target_link_libraries(${test_name} PRIVATE ${FMT_LIBRARIES})
    target_include_directories(${test_name} PUBLIC ${acme_lw_SOURCE_DIR})
    target_link_libraries(${test_name} PRIVATE acme_lw)
    target_include_directories(${test_name} PRIVATE include)
    target_include_directories(${test_name} PRIVATE src)
    target_link_libraries(${test_name} PRIVATE ${PQXX_LIBRARIES})
    add_test(NAME ${test_name} COMMAND ${test_name})
    message("NAME ${test_name} source_file ${source_file}")
endfunction()

add_test_binary(tests src/tests.cpp)

add_test_binary(tests2 src/tests2.cpp)

add_test_binary(test_engine_c_str src/test_engine_c_str.cpp)