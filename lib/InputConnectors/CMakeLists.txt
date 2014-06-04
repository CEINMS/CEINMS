## Authors: Monica Reggiani, Claudio Pizzolato

set(FromFile_SOURCES
  FromFile/EMGFromFile.cpp
  FromFile/ExternalTorqueFromFile.cpp
  FromFile/LmtMaFromFile.cpp
)
set(FromFile_HEADERS
  FromFile/EMGFromFile.h
  FromFile/ExternalTorqueFromFile.h
  FromFile/LmtMaFromFile.h
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
  ExternalTorqueFromX.cpp
  LmtMaFromX.cpp
  InputQueues.cpp
)
set(InputConnectors_HEADERS
  EMGFromX.h
  ExternalTorqueFromX.h
  LmtMaFromX.h
  InputQueues.h
)

source_group("Headers" FILES ${InputConnectors_HEADERS} ${FromFile_HEADERS} )
source_group("Sources" FILES ${InputConnectors_SOURCES} ${FromFile_SOURCES} )

include_directories(${ceinms_common_INCLUDE_DIRS} ${xmlInput_INCLUDE_DIRS} ${FileIO_INCLUDE_DIRS})
include_directories(${Boost_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/FromFile ${CMAKE_SOURCE_DIR}/lib/Concurrency)

if(OPENSIM_FOUND)
  include_directories(${OPENSIM_INCLUDE_DIRS} ${SIMTK_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR}/FromOpenSim)
  source_group("Headers" FILES  ${FromOpenSim_HEADERS})
  source_group("Sources" FILES  ${FromOpenSim_SOURCES})
endif()

add_library(InputConnectors STATIC
  ${InputConnectors_SOURCES}
  ${FromFile_SOURCES}
  ${InputConnectors_HEADERS}
  ${FromFile_HEADERS}
  ${CMAKE_SOURCE_DIR}/lib/Concurrency/Latch.cpp
)

target_link_libraries(InputConnectors FileIO)

if(OPENSIM_FOUND)
    add_library(InputConnectorsOSim STATIC
      ${InputConnectors_SOURCES}
      ${FromFile_SOURCES}
      ${FromOpenSim_SOURCES}
      ${InputConnectors_HEADERS}
      ${FromFile_HEADERS}
      ${FromOpenSim_HEADERS}
    )

    target_link_libraries(InputConnectorsOSim FileIO ${OPENSIM_LIBRARIES})

    set(InputConnectors_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/FromFile ${CMAKE_CURRENT_SOURCE_DIR}/FromOpenSim CACHE STRING "Include directories for InputConnectors library" FORCE)
else()
  set(InputConnectors_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/FromFile  CACHE STRING "Include directories for InputConnectors library" FORCE)
endif()
