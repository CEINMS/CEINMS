## Authors: Monica Reggiani, Claudio Pizzolato

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

include_directories(${ceinms_common_INCLUDE_DIRS} ${xmlInput_INCLUDE_DIRS} ${FileIO_INCLUDE_DIRS})
include_directories(${Boost_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/FromStorageFile ${CMAKE_SOURCE_DIR}/lib/Concurrency)

if(OPENSIM_FOUND)
  include_directories(${OPENSIM_INCLUDE_DIRS} ${SIMTK_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR}/FromOpenSim)
  source_group("Headers" FILES  ${FromOpenSim_HEADERS})
  source_group("Sources" FILES  ${FromOpenSim_SOURCES})
endif()

add_library(InputConnectors STATIC
  ${InputConnectors_SOURCES}
  ${FromStorageFile_SOURCES}
  ${InputConnectors_HEADERS}
  ${FromStorageFile_HEADERS}
  ${CMAKE_SOURCE_DIR}/lib/Concurrency/Latch.cpp
)

target_link_libraries(InputConnectors FileIO)

if(OPENSIM_FOUND)
    add_library(InputConnectorsOSim STATIC
      ${InputConnectors_SOURCES}
      ${FromStorageFile_SOURCES}
      ${FromOpenSim_SOURCES}
      ${InputConnectors_HEADERS}
      ${FromStorageFile_HEADERS}
      ${FromOpenSim_HEADERS}
    )

    target_link_libraries(InputConnectorsOSim FileIO ${OPENSIM_LIBRARIES})

    set(InputConnectors_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/FromStorageFile ${CMAKE_CURRENT_SOURCE_DIR}/FromOpenSim CACHE STRING "Include directories for InputConnectors library" FORCE)
else()
  set(InputConnectors_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/FromStorageFile  CACHE STRING "Include directories for InputConnectors library" FORCE)
endif()

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
