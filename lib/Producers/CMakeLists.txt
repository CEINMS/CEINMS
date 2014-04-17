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

set(Producers_SOURCES
  EMGFromX.cpp
  ExternalTorqueFromX.cpp
  LmtMaFromX.cpp
  Semaphore.cpp
  SyncTools.cpp
)
set(Producers_HEADERS
  EMGFromX.h
  ExternalTorqueFromX.h
  LmtMaFromX.h
  Semaphore.h
  SyncTools.h
)

source_group("Headers" FILES ${Producers_HEADERS} ${FromFile_HEADERS} )
source_group("Sources" FILES ${Producers_SOURCES} ${FromFile_SOURCES} )

include_directories(${ceinms_common_INCLUDE_DIRS} ${xmlInput_INCLUDE_DIRS} ${FileIO_INCLUDE_DIRS})
include_directories(${Boost_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/FromFile)

if(OPENSIM_FOUND)
  include_directories(${OPENSIM_INCLUDE_DIRS} ${SIMTK_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR}/FromOpenSim)
  source_group("Headers" FILES  ${FromOpenSim_HEADERS})
  source_group("Sources" FILES  ${FromOpenSim_SOURCES})
endif()

add_library(Producers STATIC
  ${Producers_SOURCES}
  ${FromFile_SOURCES}
  ${Producers_HEADERS}
  ${FromFile_HEADERS}
)

target_link_libraries(Producers FileIO)

if(OPENSIM_FOUND)
    add_library(ProducersOSim STATIC
      ${Producers_SOURCES}
      ${FromFile_SOURCES}
      ${FromOpenSim_SOURCES}
      ${Producers_HEADERS}
      ${FromFile_HEADERS}
      ${FromOpenSim_HEADERS}
    )

    target_link_libraries(ProducersOSim FileIO ${OPENSIM_LIBRARIES})

    set(Producers_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/FromFile ${CMAKE_CURRENT_SOURCE_DIR}/FromOpenSim CACHE STRING "Include directories for Producers library" FORCE)
else()
  set(Producers_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/FromFile  CACHE STRING "Include directories for Producers library" FORCE)
endif()

