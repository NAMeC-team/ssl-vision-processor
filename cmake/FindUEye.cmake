if (WITH_UEYE)

    # Search for IDS uEye / IDS peak headers
    find_path(UEYE_INCLUDE_DIRS
        NAMES ueye.h peak_icv.hpp
        HINTS
            ${UEYE_IDS_PEAK_PEAK_INCLUDE_DIR}
            ${UEYE_IDS_PEAK_PEAK_ICV_INCLUDE_DIR}
            /opt/ueye/include
            /usr/include/ueye
            /usr/local/include/ueye
            /opt/ids_peak/include
            /usr/include
            /usr/local/include
    )

    # Search for IDS uEye / IDS peak libraries
    find_library(UEYE_LIBS
        NAMES
            ueye_api
            ids_peak
        HINTS
            ${UEYE_IDS_PEAK_LIB_DIR}
            /opt/ueye/lib
            /usr/lib
            /usr/local/lib
            /opt/ids_peak/lib
    )

    if (UEYE_INCLUDE_DIRS AND UEYE_LIBS)
        message(STATUS "IDS uEye / IDS peak SDK found.")
        message(STATUS "Include directory: ${UEYE_INCLUDE_DIRS}")
        message(STATUS "Library: ${UEYE_LIBS}")
        add_definitions(-DUEYE)
    else()
        message(WARNING "IDS uEye / IDS peak SDK not found.")
        message(WARNING "Include directory: ${UEYE_INCLUDE_DIRS}")
        message(WARNING "Library: ${UEYE_LIBS}")
        set(UEYE_INCLUDE_DIRS "")
        set(UEYE_LIBS "")
    endif()

endif()