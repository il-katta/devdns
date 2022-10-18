find_package(PkgConfig)
pkg_check_modules(PC_PQXX QUIET pqxx)
set(PQXX_DEFINITIONS ${PC_PQXX_CFLAGS_OTHER})

find_path(
        PQXX_INCLUDE_DIR pqxx
        HINTS ${PC_PQXX_INCLUDEDIR} ${PC_PQXX_INCLUDE_DIRS}
        PATH_SUFFIXES pqxx
)

find_library(PQXX_LIBRARY NAMES pqxx HINTS ${PC_PQXX_LIBDIR} ${PC_PQXX_LIBRARY_DIRS})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(PQXX DEFAULT_MSG PQXX_LIBRARY PQXX_INCLUDE_DIR)
mark_as_advanced(PQXX_INCLUDE_DIR PQXX_LIBRARY)

set(PQXX_LIBRARIES ${PQXX_LIBRARY})
set(PQXX_INCLUDE_DIRS ${PQXX_INCLUDE_DIR})