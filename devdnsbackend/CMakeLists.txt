include(FetchContent)
include(ExternalProject)

cmake_minimum_required(VERSION 3.8)
set(CMAKE_CXX_STANDARD 17)

set(LIB_NAME "devdnsbackend")
set(PDNS_TAG "auth-4.7.2")
set(DEPENDENCIES)
set(PQXX_TAG "7.7.4")

#set(CMAKE_CXX_FLAGS "-Wno-deprecated-declarations")

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

project(${LIB_NAME})

# fmt dep
find_library(FMT_LIBRARIES fmt REQUIRED)

# pq
find_library(PQ_LIBRARY pq REQUIRED)
find_package(gpgsqlbackend REQUIRED)
find_path(PQ_INCLUDE_DIR postgresql/libpq-fe.h)

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

# OpenSSL

#find_package(OpenSSL 1.0...<3.0 REQUIRED)
find_package(OpenSSL REQUIRED)
message("openssl " ${OPENSSL_VERSION} " " ${OPENSSL_INCLUDE_DIR} " " ${OPENSSL_LIBRARIES})
# pdns headers
FetchContent_Declare(
    pdns
    GIT_REPOSITORY https://github.com/PowerDNS/pdns.git
    GIT_TAG ${PDNS_TAG}
)
FetchContent_MakeAvailable(pdns)
FetchContent_GetProperties(pdns)
IF (NOT EXISTS ${pdns_SOURCE_DIR}/configure)
    exec_program(
        autoreconf ${pdns_SOURCE_DIR}
        ARGS -vi
    )
ENDIF ()
IF (NOT EXISTS ${pdns_SOURCE_DIR}/config.h)
    exec_program(
            ./configure ${pdns_SOURCE_DIR}
            ARGS --prefix=/usr
                --sysconfdir=/etc/powerdns
                --sbindir=/usr/bin
                --with-modules=''
                --with-dynmodules="gpgsql"
                --with-libsodium
                --enable-tools
                --enable-ixfrdist
                --enable-dns-over-tls
                --disable-dependency-tracking
                --disable-silent-rules
                --enable-reproducible
                --enable-unit-tests
                --disable-systemd
                --with-service-user=powerdns
                --with-service-group=powerdns
    )
    exec_program(
        sed ${pdns_SOURCE_DIR}
        ARGS -i 's/EVP_MD_CTX_destroy/EVP_MD_CTX_free/g' pdns/sha.hh
    )
ENDIF ()

add_library(${LIB_NAME} MODULE src/${LIB_NAME}.cpp)
target_include_directories(${LIB_NAME} PRIVATE include)
target_include_directories(${LIB_NAME} PRIVATE src)
# openssl
target_include_directories(${LIB_NAME} PRIVATE ${OPENSSL_INCLUDE_DIR})
target_link_libraries(${LIB_NAME} PRIVATE ${OPENSSL_LIBRARIES})
# fmt
target_link_libraries(${LIB_NAME} PRIVATE ${FMT_LIBRARIES})
# pq
target_link_libraries(${LIB_NAME} PRIVATE ${PQ_LIBRARY} ${GPSQLBACKEND_LIBRARY})
target_include_directories(${LIB_NAME} PRIVATE ${PQ_INCLUDE_DIR}/postgresql)
# pqxx
target_link_libraries(${LIB_NAME} PRIVATE ${PQXX_LIBRARIES})
# pdns headers
target_include_directories(${LIB_NAME} PRIVATE ${pdns_SOURCE_DIR})

# install
install(TARGETS ${LIB_NAME} DESTINATION lib/pdns)

# tests
project(tests)
enable_testing()

function(add_test_binary test_name source_file)
    add_executable(${test_name} ${source_file})
    target_include_directories(${test_name} PRIVATE ${pdns_SOURCE_DIR})
    target_link_libraries(${test_name} PRIVATE ${FMT_LIBRARIES})
    target_include_directories(${test_name} PRIVATE include)
    target_include_directories(${test_name} PRIVATE src)
    target_link_libraries(${test_name} PRIVATE ${PQXX_LIBRARIES})
    add_test(NAME ${test_name} COMMAND ${test_name})
    message("NAME ${test_name} source_file ${source_file}")
endfunction()

add_test_binary(tests src/tests.cpp)

add_test_binary(tests2 src/tests2.cpp)

add_test_binary(test_engine_c_str src/test_engine_c_str.cpp)