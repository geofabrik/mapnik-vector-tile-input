cmake_minimum_required(VERSION 3.5.0)
project(mapnik-mbtiles-vector VERSION 0.0.1 LANGUAGES CXX C)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(AUTHOR "Michael Reichert <michael.reichert@geofabrik.de>")


#-----------------------------------------------------------------------------
#
#  Find external dependencies
#
#-----------------------------------------------------------------------------

find_package(Mapnik REQUIRED)
find_package(Protozero REQUIRED)
find_package(SQLite3 REQUIRED)
find_package(ZLIB REQUIRED)
find_package(Boost 1.55.0 REQUIRED)

#-----------------------------------------------------------------------------
#
#  Decide which C++ version to use (Minimum/default: C++17).
#
#-----------------------------------------------------------------------------
if(NOT MSVC)
    if(NOT USE_CPP_VERSION)
        set(USE_CPP_VERSION c++17)
    endif()
    message(STATUS "Use C++ version: ${USE_CPP_VERSION}")
    # following only available from cmake 2.8.12:
    #   add_compile_options(-std=${USE_CPP_VERSION})
    # so using this instead:
    add_definitions(-std=${USE_CPP_VERSION})
endif()


#-----------------------------------------------------------------------------
#
#  Compiler and Linker flags
#
#-----------------------------------------------------------------------------
set(USUAL_COMPILE_OPTIONS "-O3 -g")

set(CMAKE_CXX_FLAGS_DEV "${USUAL_COMPILE_OPTIONS}"
    CACHE STRING "Flags used by the compiler during developer builds."
    FORCE)

set(CMAKE_EXE_LINKER_FLAGS_DEV ""
    CACHE STRING "Flags used by the linker during developer builds."
    FORCE)
mark_as_advanced(
    CMAKE_CXX_FLAGS_DEV
    CMAKE_EXE_LINKER_FLAGS_DEV
)

set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${USUAL_COMPILE_OPTIONS}"
    CACHE STRING "Flags used by the compiler during RELWITHDEBINFO builds."
    FORCE)


#-----------------------------------------------------------------------------
#
#  Build Type
#
#-----------------------------------------------------------------------------
set(CMAKE_CONFIGURATION_TYPES "Debug Release RelWithDebInfo MinSizeRel Dev")

# In 'Dev' mode: compile with very strict warnings and turn them into errors.
if(CMAKE_BUILD_TYPE STREQUAL "Dev")
    if(NOT MSVC)
        add_definitions(-Werror -fno-omit-frame-pointer)
    endif()
endif()

# Force RelWithDebInfo build type if none was given
if(CMAKE_BUILD_TYPE)
    set(build_type ${CMAKE_BUILD_TYPE})
else()
    set(build_type "RelWithDebInfo")
endif()

set(CMAKE_BUILD_TYPE ${build_type}
    CACHE STRING
    "Choose the type of build, options are: ${CMAKE_CONFIGURATION_TYPES}."
    FORCE)

#-----------------------------------------------------------------------------
#
#  Install Path for the Plugin
#
#-----------------------------------------------------------------------------
# Install path
if (NOT MAPNIK_PLUGIN_INSTALL_DIR)
    execute_process(COMMAND mapnik-config --input-plugins
        OUTPUT_VARIABLE MAPNIK_CONFIG_PLUGIN_PATH
        OUTPUT_STRIP_TRAILING_WHITESPACE)
    if (NOT MAPNIK_CONFIG_PLUGIN_PATH)
        message(FATAL_ERROR "Mapnik plugin path could not be found. Set the path with -DMAPNIK_PLUGIN_INSTALL_DIR.")
    endif()
endif()
set(MAPNIK_PLUGIN_INSTALL_DIR ${MAPNIK_CONFIG_PLUGIN_PATH} CACHE STRING "Install directory for Mapnik input plugins")
message(STATUS "Mapnik plugin path: ${MAPNIK_PLUGIN_INSTALL_DIR}")

add_subdirectory(src)

