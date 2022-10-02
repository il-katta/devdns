set(GPSQLBACKEND_SEARCH_PATHS
        /usr/lib/powerdns
        /usr/lib/pdns
        /usr/local/lib/pdns
        /usr/local/lib/powerdns
        /usr/lib64/powerdns
        /usr/lib64/pdns
        /usr/local/lib64/pdns
        /usr/local/lib64/powerdns
        )

find_library(GPSQLBACKEND_LIBRARY NAMES gpgsqlbackend HINTS PATHS ${GPSQLBACKEND_SEARCH_PATHS})
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(gpgsqlbackend REQUIRED_VARS GPSQLBACKEND_LIBRARY)