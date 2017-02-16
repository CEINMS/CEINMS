# Authors: Monica Reggiani, Claudio Pizzolato

cmake_minimum_required (VERSION 3.0.0) # need add_library(... INTERFACE)
project(CEINMS)
set(CEINMS_MAJOR_VERSION 0)
set(CEINMS_MINOR_VERSION 9)
set(CEINMS_PATCH_VERSION 0)

# Include GNUInstallDirs to get canonical paths
include(GNUInstallDirs)

if(WIN32)
    set(CMAKE_CXX_FLAGS "/EHsc")# -std=c++11 -fpermissive")
endif()
if(UNIX AND NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel." FORCE)
#	set(CMAKE_CXX_FLAGS "-O3")
#	set(CMAKE_EXE_LINKER_FLAGS "-O3")
#	set(CMAKE_SHARED_LINKER_FLAGS "-O3")
endif()

if(UNIX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -std=c++11 -Wno-deprecated-declarations ")
endif(UNIX)

set(CMAKE_VERBOSE_MAKEFILE OFF)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)
include(CeinmsPack)

set(Boost_USE_STATIC_LIBS   ON)
set(Boost_USE_MULTITHREADED ON)
find_package(Boost REQUIRED COMPONENTS filesystem system)

find_package(XSD REQUIRED)

find_package(XERCES REQUIRED)

find_package(Concurrency REQUIRED)

# Choose common build locations for executables and libraries
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

if(MSVC)
    set(CMAKE_DEBUG_POSTFIX "_d")
endif(MSVC)

option(CEINMS_VERBOSE_DEBUG "Print debug messages" OFF)
option(CEINMS_VERBOSE_DEEP_DEBUG "Print LOTS of debug messages" OFF)
if (CEINMS_VERBOSE_DEBUG)
    add_definitions(/DDEBUG)
endif()

if (CEINMS_VERBOSE_DEEP_DEBUG)
    add_definitions(/DDEEP_DEBUG)
endif()

add_definitions(/D_USE_MATH_DEFINES)

add_subdirectory(lib)
add_subdirectory(src)
add_subdirectory(docs)
# Create CEINMSConfig.cmake etc.
set(ConfigPackageLocation ${CMAKE_INSTALL_LIBDIR}/CEINMS)
install(EXPORT CEINMSTargets
        FILE CEINMSTargets.cmake
        NAMESPACE CEINMS::
        DESTINATION ${ConfigPackageLocation})

include(CMakePackageConfigHelpers)
configure_package_config_file(cmake/CEINMSConfig.cmake.in
                              ${CMAKE_CURRENT_BINARY_DIR}/CEINMSConfig.cmake
                              INSTALL_DESTINATION ${ConfigPackageLocation}
                              PATH_VARS CMAKE_INSTALL_INCLUDEDIR)

write_basic_package_version_file(${CMAKE_CURRENT_BINARY_DIR}/CEINMSConfigVersion.cmake
                                    VERSION ${CEINMS_MAJOR_VERSION}.${CEINMS_MINOR_VERSION}.${CEINMS_PATCH_VERSION}
                                    COMPATIBILITY ExactVersion )

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/CEINMSConfig.cmake
              ${CMAKE_CURRENT_BINARY_DIR}/CEINMSConfigVersion.cmake
              DESTINATION ${ConfigPackageLocation})

# Uninstall target
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/cmake_uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
    IMMEDIATE @ONLY)

add_custom_target(uninstall
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)
