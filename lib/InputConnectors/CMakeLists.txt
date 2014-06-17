## Authors: Monica Reggiani, Claudio Pizzolato, Elena Ceseracciu

# List source files
set(FromStorageFile_SOURCES
  FromStorageFile/EMGFromFile.cpp
  FromStorageFile/ExternalTorquesFromStorageFile.cpp
  FromStorageFile/LmtMaFromStorageFile.cpp
  FromStorageFile/Utilities.cpp
)
set(FromStorageFile_HEADERS
  FromStorageFile/EMGFromFile.h
  FromStorageFile/ExternalTorquesFromStorageFile.h
  FromStorageFile/LmtMaFromStorageFile.h
  FromStorageFile/Utilities.h
)

set(FromOpenSim_SOURCES
  FromOpenSim/LmtMaFromOpenSim.cpp
  FromOpenSim/ExternalTorqueFromStorage.cpp
)
set(FromOpenSim_HEADERS
  FromOpenSim/LmtMaFromOpenSim.h
  FromOpenSim/ExternalTorqueFromStorage.h
)

set(InputConnectors_SOURCES
  EMGFromX.cpp
  ExternalTorquesFromX.cpp
  LmtMaFromX.cpp
  InputQueues.cpp
)
set(InputConnectors_HEADERS
  EMGFromX.h
  ExternalTorquesFromX.h
  LmtMaFromX.h
  InputQueues.h
)

source_group("Headers" FILES ${InputConnectors_HEADERS} ${FromStorageFile_HEADERS} )
source_group("Sources" FILES ${InputConnectors_SOURCES} ${FromStorageFile_SOURCES} )
if(OPENSIM_FOUND)
  source_group("Headers" FILES  ${FromOpenSim_HEADERS})
  source_group("Sources" FILES  ${FromOpenSim_SOURCES})
endif()

# Define targets
add_library(InputConnectors STATIC
  ${InputConnectors_SOURCES}
  ${FromStorageFile_SOURCES}
  ${InputConnectors_HEADERS}
  ${FromStorageFile_HEADERS}
)
target_link_libraries(InputConnectors FileIO Concurrency ${Boost_FILESYSTEM_LIBRARY})
target_include_directories(InputConnectors
                                PRIVATE     FromStorageFile
                                            ${CMAKE_CURRENT_SOURCE_DIR}
                                            ${ceinms_common_INCLUDE_DIRS}
                                            ${FileIO_INCLUDE_DIRS}
                                            ${Concurrency_INCLUDE_DIRS}
                                            ${Boost_INCLUDE_DIRS})


if(OPENSIM_FOUND)
    add_library(InputConnectorsOSim STATIC
      ${InputConnectors_SOURCES}
      ${FromStorageFile_SOURCES}
      ${FromOpenSim_SOURCES}
      ${InputConnectors_HEADERS}
      ${FromStorageFile_HEADERS}
      ${FromOpenSim_HEADERS}
    )

    target_link_libraries(InputConnectorsOSim FileIO Concurrency ${Boost_FILESYSTEM_LIBRARY} ${OPENSIM_LIBRARIES})
    target_include_directories(InputConnectors
                                PRIVATE     FromStorageFile
                                            FromOpenSim
                                            ${CMAKE_CURRENT_SOURCE_DIR}
                                            ${ceinms_common_INCLUDE_DIRS}
                                            ${FileIO_INCLUDE_DIRS}
                                            ${Concurrency_INCLUDE_DIRS}
                                            ${Boost_INCLUDE_DIRS}
                                            FromOpenSim
                                PUBLIC      ${OPENSIM_INCLUDE_DIRS} ${SIMTK_INCLUDE_DIRS})
endif()

# Save path to include directory for "internal" usage
if(OPENSIM_FOUND)
    set(InputConnectors_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/FromStorageFile" "${CMAKE_CURRENT_SOURCE_DIR}/FromOpenSim" CACHE STRING "Include directories for InputConnectors library" FORCE)
else()
    set(InputConnectors_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/FromStorageFile  CACHE STRING "Include directories for InputConnectors library" FORCE)
endif()
mark_as_advanced(InputConnectors_INCLUDE_DIRS)

# Install
install(FILES ${FromStorageFile_HEADERS} ${InputConnectors_HEADERS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}) #should add namespace
install(TARGETS InputConnectors
        EXPORT CEINMSTargets
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})

if(OPENSIM_FOUND)
    install(FILES ${FromOpenSim_HEADERS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
    install(TARGETS InputConnectorsOSim
        EXPORT CEINMSTargets
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
endif()
