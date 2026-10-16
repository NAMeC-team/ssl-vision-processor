if (WITH_UEYE)

    # Search for IDS uEye / IDS peak headers
    find_path(UEYE_INCLUDE_DIRS
        NAMES ueye.h ids_peak.h
        HINTS
            ${CMAKE_SOURCE_DIR}
            ${CMAKE_SOURCE_DIR}/include
            ${CMAKE_SOURCE_DIR}/include/ueye
            ${CMAKE_SOURCE_DIR}/include/ids_peak
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
            ${CMAKE_SOURCE_DIR}
            ${CMAKE_SOURCE_DIR}/lib
            ${CMAKE_SOURCE_DIR}/lib/ueye
            ${CMAKE_SOURCE_DIR}/lib/ids_peak
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
        set(UEYE_INCLUDE_DIRS "")
        set(UEYE_LIBS "")
    endif()

endif()