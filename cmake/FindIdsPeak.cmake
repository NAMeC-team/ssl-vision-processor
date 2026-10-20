if (WITH_IDS_PEAK)

set(CMAKE_FIND_DEBUG_MODE TRUE)
    find_path(IDS_PEAK_INCLUDE_DIR
        NAMES peak/peak.hpp ids_peak_icv/peak_icv/peak_icv.hpp
        PATHS
            /opt/ids/ids-peak/include/ids-peak
            # ${IDS_PEAK_DIR}/include/ids-peak
            /usr/include/ueye
            /usr/local/include/ueye
            /opt/ids_peak/include
            /usr/include
            /usr/local/include
    )
    find_path(IDS_PEAK_COMMON_INCLUDE_DIR
        NAMES peak_common/peak_common.hpp
        PATHS
            /opt/ids/ids-peak/include/
            /usr/include/ueye
            /usr/local/include/ueye
            /opt/ids_peak/include
            /usr/include
            /usr/local/include
    )

    find_library(IDS_PEAK_LIBS
        NAMES
            ids_peak
        HINTS
            /opt/ids/ids-peak/lib/x86_64-linux-gnu/
            /opt/ueye/lib
            /usr/lib
            /usr/local/lib
            /opt/ids_peak/lib
    )

    if (IDS_PEAK_INCLUDE_DIR AND IDS_PEAK_LIBS)
        message(STATUS "IDS uEye / IDS peak SDK found.")
        add_definitions(-DIDS_PEAK)
    else()
        message(WARNING "IDS uEye / IDS peak SDK not found.")
        message(WARNING "Include directory: ${IDS_PEAK_INCLUDE_DIR}")
        message(WARNING "Library: ${IDS_PEAK_LIB}")
        set(IDS_PEAK_INCLUDE_DIR "")
        set(IDS_PEAK_LIB "")
    endif()

    set(CMAKE_FIND_DEBUG_MODE FALSE)
endif()
