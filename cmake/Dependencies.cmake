include(FetchContent)

# ---------------------------------------------------------------------------
# Catch2 v3 (Unit Testing Framework)
# ---------------------------------------------------------------------------
if(BUILD_TESTS)
    FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG        v3.9.1
        GIT_SHALLOW    TRUE
    )
    FetchContent_MakeAvailable(Catch2)
endif()

# ---------------------------------------------------------------------------
# lwgps (Lightweight GPS NMEA parser)
# ---------------------------------------------------------------------------
FetchContent_Declare(
    lwgps
    GIT_REPOSITORY https://github.com/MaJerle/lwgps.git
    GIT_TAG        v2.4.0
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(lwgps)

# ---------------------------------------------------------------------------
# Fusion (xioTechnologies AHRS sensor fusion algorithm)
# ---------------------------------------------------------------------------
FetchContent_Declare(
    fusion
    GIT_REPOSITORY https://github.com/xioTechnologies/Fusion.git
    GIT_TAG        v1.3.3
    GIT_SHALLOW    TRUE
)
FetchContent_GetProperties(fusion)
if(NOT fusion_POPULATED)
    FetchContent_Populate(fusion)
    # Add only the core Fusion C library, avoiding Python/examples
    add_subdirectory(${fusion_SOURCE_DIR}/Fusion ${fusion_BINARY_DIR}/Fusion EXCLUDE_FROM_ALL)
endif()

