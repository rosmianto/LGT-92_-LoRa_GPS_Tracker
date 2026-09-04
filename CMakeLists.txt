cmake_minimum_required(VERSION 3.22)

project(LGT92
    VERSION 2.0.0
    DESCRIPTION "Modernized C++20 Firmware for Dragino LGT-92 LoRa GPS Tracker"
    LANGUAGES C CXX ASM
)

# ---------------------------------------------------------------------------
# Language Standards
# ---------------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ---------------------------------------------------------------------------
# Build Options
# ---------------------------------------------------------------------------
option(BUILD_TESTS "Build host unit tests with Catch2" ON)
option(BUILD_WASM  "Build WebAssembly simulator with Emscripten" OFF)

# ---------------------------------------------------------------------------
# Dependencies (Catch2, lwgps, Fusion)
# ---------------------------------------------------------------------------
include(cmake/Dependencies.cmake)

# ---------------------------------------------------------------------------
# Core Domain & Adapters Library (Hardware Independent)
# ---------------------------------------------------------------------------
add_library(lgt92_core
    src/core/TrackerConfig.cpp
    src/core/Payload.cpp
    src/core/DownlinkParser.cpp
    src/core/TrackerEngine.cpp
    src/adapters/LwGpsAdapter.cpp
    src/adapters/FusionAdapter.cpp
)

target_include_directories(lgt92_core PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(lgt92_core PUBLIC
    lwgps
    Fusion
)

target_compile_features(lgt92_core PUBLIC cxx_std_20)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    target_compile_options(lgt92_core PRIVATE
        -Wall
        -Wextra
        -Wpedantic
        -Wshadow
        -Wconversion
        -Wnon-virtual-dtor
    )
endif()

# ---------------------------------------------------------------------------
# Subdirectories
# ---------------------------------------------------------------------------
if(BUILD_TESTS AND NOT CMAKE_CROSSCOMPILING)
    enable_testing()
    add_subdirectory(tests)
endif()

if(BUILD_WASM)
    add_subdirectory(sim)
endif()

if(CMAKE_CROSSCOMPILING AND CMAKE_SYSTEM_NAME STREQUAL "Generic")
    add_subdirectory(target/stm32)
endif()
