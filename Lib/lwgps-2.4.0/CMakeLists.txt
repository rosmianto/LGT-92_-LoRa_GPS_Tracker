cmake_minimum_required(VERSION 3.22)

if(DEFINED ESP_PLATFORM)
    set(LWGPS_CUSTOM_INC_DIR ${CMAKE_CURRENT_BINARY_DIR}/lib_inc)

    if(NOT LWGPS_OPTS_FILE)
        message(STATUS "Using default lwgps_opts.h file")
        set(LWGPS_OPTS_FILE ${CMAKE_CURRENT_LIST_DIR}/lwgps/src/include/lwgps/lwgps_opts_template.h)
    else()
        message(STATUS "Using custom lwgps_opts.h file from ${LWGPS_OPTS_FILE}")
    endif()

    configure_file(${LWGPS_OPTS_FILE} ${LWGPS_CUSTOM_INC_DIR}/lwgps_opts.h COPYONLY)

    idf_component_register(
        SRCS "lwgps/src/lwgps/lwgps.c"
        INCLUDE_DIRS "lwgps/src/include" ${LWGPS_CUSTOM_INC_DIR}
    )
    return()
endif()

# Standalone / subdirectory build
project(LwLibPROJECT)

if(NOT PROJECT_IS_TOP_LEVEL)
    add_subdirectory(lwgps)
else()
    set(TEST_CMAKE_FILE_NAME "test_parse_sat_det/cmake.cmake")
    add_subdirectory(tests)
endif()
