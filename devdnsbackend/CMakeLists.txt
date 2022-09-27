include(FetchContent)

cmake_minimum_required(VERSION 3.6)
set(LIB_NAME "devdnsbackend")

project(${LIB_NAME})

SET(PDNS_TAG "auth-4.6.3")
set(CMAKE_CXX_STANDARD 17)

add_library(${LIB_NAME} MODULE src/${LIB_NAME}.cpp)
target_include_directories(${LIB_NAME} PRIVATE include)
target_include_directories(${LIB_NAME} PRIVATE src)

# ftm dep
find_library(FMT_LIBRARIES fmt REQUIRED)
target_link_libraries(${LIB_NAME} ${FMT_LIBRARIES})

# pdns headers
FetchContent_Declare(
        pdns
        GIT_REPOSITORY https://github.com/PowerDNS/pdns.git
        GIT_TAG ${PDNS_TAG}
)
FetchContent_MakeAvailable(pdns)
FetchContent_GetProperties(pdns)
if (NOT pdns_POPULATED)
    FetchContent_Populate(pdns)
endif ()
target_include_directories(${LIB_NAME} PRIVATE ${pdns_SOURCE_DIR})

# install
install(TARGETS ${LIB_NAME} DESTINATION lib/pdns)
