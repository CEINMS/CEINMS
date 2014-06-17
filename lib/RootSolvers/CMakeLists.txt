# Author: Elena Ceseracciu

# Save path to include directory for "internal" usage
set(RootSolvers_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR} CACHE STRING "Include directory for RootSolvers library")
mark_as_advanced(RootSolvers_INCLUDE_DIRS)

# Install
#install(FILES ${_HEADERS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}) #should add namespace
