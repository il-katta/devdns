cmake_minimum_required(VERSION 3.6)
set(LIB_NAME "devdnsbackend")
include(FetchContent)
project(${LIB_NAME})
SET(PDNS_TAG "auth-4.6.3")
set(CMAKE_CXX_STANDARD 17)

find_library(FMT_LIBRARIES fmt REQUIRED)
FetchContent_Declare(
        pdns
        GIT_REPOSITORY https://github.com/PowerDNS/pdns.git
        GIT_TAG ${PDNS_TAG}
)
set(PDNS_LIBRARIES pdns)
FetchContent_MakeAvailable(pdns)
FetchContent_GetProperties(pdns)
if (NOT pdns_POPULATED)
    FetchContent_Populate(pdns)
endif ()
#set(CMAKE_MODULE_LINKER_FLAGS "-module -avoid-version")
add_library(${LIB_NAME} MODULE src/${LIB_NAME}.cpp)
target_link_libraries(${LIB_NAME} ${FMT_LIBRARIES})
#SET_TARGET_PROPERTIES(${LIB_NAME} PROPERTIES LINKER_FLAGS "-module -avoid-version")
target_include_directories(${LIB_NAME} PRIVATE include)
target_include_directories(${LIB_NAME} PRIVATE src)
target_include_directories(${LIB_NAME} PRIVATE ${pdns_SOURCE_DIR})

install(TARGETS ${LIB_NAME} DESTINATION lib/pdns)

#ENABLE_TESTING()
#ADD_SUBDIRECTORY(tests)

