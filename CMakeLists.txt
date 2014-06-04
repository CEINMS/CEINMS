# Authors: Monica Reggiani, Claudio Pizzolato

cmake_minimum_required (VERSION 2.8.5)
project(CEINMS)

# Include GNUInstallDirs to get canonical paths
include(GNUInstallDirs)

if(WIN32)
    set(CMAKE_CXX_FLAGS "/EHsc")# -std=c++11 -fpermissive")
endif()
if(UNIX AND NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel." FORCE)
#	set(CMAKE_CXX_FLAGS "-O3")
#	set(CMAKE_EXE_LINKER_FLAGS "-O3")
#	set(CMAKE_SHARED_LINKER_FLAGS "-O3")
endif()

if(UNIX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -std=c++11 -Wno-deprecated-declarations ")
endif(UNIX)

set(CMAKE_VERBOSE_MAKEFILE OFF)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR})

set(Boost_USE_STATIC_LIBS   ON)
set(Boost_USE_MULTITHREADED ON)
find_package(Boost REQUIRED COMPONENTS thread date_time program_options chrono filesystem system )

find_package(XSD REQUIRED)

find_package(XERCES REQUIRED)

find_package(OpenSim)
#link_directories(${OPENSIM_LIBRARY_DIRS})
#find_package(Eigen3 REQUIRED)
#include_directories(${EIGEN3_INCLUDE_DIR})

add_subdirectory(lib)
add_subdirectory(src)
#add_subdirectory(sandBox)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin CACHE PATH "Build directory" FORCE)
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/lib CACHE PATH "Build directory" FORCE)


