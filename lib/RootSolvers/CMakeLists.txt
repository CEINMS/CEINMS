# Author: Elena Ceseracciu
set(RootSolvers_HEADERS  ceinms/WDBsolver.h)

add_library(RootSolvers INTERFACE)
add_custom_target(RootSolvers_IDE SOURCES ${RootSolvers_HEADERS})
target_include_directories(RootSolvers INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)
