include(FetchContent)
include(ExternalProject)

cmake_minimum_required(VERSION 3.6)
set(CMAKE_CXX_STANDARD 17)

set(LIB_NAME "devdnsbackend")
set(PDNS_TAG "auth-4.6.3")
set(UACME_TAG "upstream/1.7.3")
set(DEPENDENCIES)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

project(${LIB_NAME})

add_library(${LIB_NAME} MODULE src/${LIB_NAME}.cpp)
target_include_directories(${LIB_NAME} PRIVATE include)
target_include_directories(${LIB_NAME} PRIVATE src)

# ftm dep
find_library(FMT_LIBRARIES fmt REQUIRED)
target_link_libraries(${LIB_NAME} ${FMT_LIBRARIES})

# uacme
find_library(UACME_LIBRARIES uacme REQUIRED)
target_link_libraries(${LIB_NAME} ${UACME_LIBRARIES})


# pq
find_library(PQ_LIBRARIES pq REQUIRED)
find_package(gpgsqlbackend REQUIRED)
target_link_libraries(${LIB_NAME} ${PQ_LIBRARIES} ${GPSQLBACKEND_LIBRARY})

# pdns headers
FetchContent_Declare(
    pdns
    GIT_REPOSITORY https://github.com/PowerDNS/pdns.git
    GIT_TAG ${PDNS_TAG}
)
FetchContent_MakeAvailable(pdns)
FetchContent_GetProperties(pdns)
target_include_directories(${LIB_NAME} PRIVATE ${pdns_SOURCE_DIR})

# uacme
FetchContent_Declare(
        uacme
        GIT_REPOSITORY https://github.com/ndilieto/uacme.git
        GIT_TAG ${UACME_TAG}
)

FetchContent_MakeAvailable(uacme)
FetchContent_GetProperties(uacme)
exec_program(
        ./configure ${uacme_SOURCE_DIR}
        ARGS --disable-maintainer-mode --disable-docs --no-recursion
)
# TODO add check to dependency libcurl gnutls ...

# acme_lw
FetchContent_Declare(
        acme_lw
        GIT_REPOSITORY https://git.loopback.it/andrea/acme-lw.git
        GIT_TAG master
)
FetchContent_MakeAvailable(acme_lw)
FetchContent_GetProperties(acme_lw)
target_include_directories(${LIB_NAME} PUBLIC ${acme_lw_SOURCE_DIR})
target_link_libraries(${LIB_NAME} acme_lw)


# install
install(TARGETS ${LIB_NAME} DESTINATION lib/pdns)

# tests
project(tests)
enable_testing()

add_executable(tests src/tests.cpp)
target_include_directories(tests PRIVATE ${pdns_SOURCE_DIR})
target_link_libraries(tests ${UACME_LIBRARIES})
target_link_libraries(tests ${FMT_LIBRARIES})
target_include_directories(tests PUBLIC ${acme_lw_SOURCE_DIR})
target_link_libraries(tests acme_lw)
target_include_directories(tests PRIVATE include)
target_include_directories(tests PRIVATE src)
add_test(NAME tests COMMAND tests)

add_executable(tests2 src/tests2.cpp)
target_include_directories(tests2 PRIVATE ${pdns_SOURCE_DIR})
target_link_libraries(tests2 ${UACME_LIBRARIES})
target_link_libraries(tests2 ${FMT_LIBRARIES})
target_include_directories(tests2 PRIVATE include)
target_include_directories(tests2 PRIVATE src)
target_include_directories(tests2 PUBLIC ${acme_lw_SOURCE_DIR})
target_link_libraries(tests2 acme_lw)
add_test(NAME tests2 COMMAND tests2)
