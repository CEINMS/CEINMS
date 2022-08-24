## Authors: Monica Reggiani, Claudio Pizzolato, Elena Ceseracciu

# List source files
set(FromStorageFile_SOURCES
  FromStorageFile/EmgFromFile.cpp
  FromStorageFile/RawEmgFromFile.cpp
  FromStorageFile/ExternalTorquesFromStorageFile.cpp
  FromStorageFile/LmtMaFromStorageFile.cpp
  FromStorageFile/Utilities.cpp
  FromStorageFile/ExternalForcesFromStorageFile.cpp
  FromStorageFile/GeneralisedCoordinatesFromStorageFile.cpp
)
set(FromStorageFile_HEADERS
  ceinms/EmgFromFile.h
  ceinms/RawEmgFromFile.h
  ceinms/ExternalTorquesFromStorageFile.h
  ceinms/LmtMaFromStorageFile.h
  ceinms/Utilities.h
  ceinms/ExternalForcesFromStorageFile.h
  ceinms/GeneralisedCoordinatesFromStorageFile.h
  ceinms/EmgFromQueue.h
)

set(InputConnectors_SOURCES
  EmgFromX.cpp
  ExternalTorquesFromX.cpp
  LmtMaFromX.cpp
  ExternalForcesFromX.cpp
  GeneralisedCoordinatesFromX.cpp 
  EmgFromQueue.cpp
  #LmtMaFromMcbs.cpp
)
set(InputConnectors_HEADERS
  ceinms/EmgFromX.h
  ceinms/ExternalTorquesFromX.h
  ceinms/LmtMaFromX.h
  ceinms/ExternalForcesFromX.h
  ceinms/GeneralisedCoordinatesFromX.h
 # ceinms/LmtMaFromMcbs.h
)

set(InputConnectors_TEMPLATE_IMPLEMENTATIONS)

set_source_files_properties(${InputConnectors_TEMPLATE_IMPLEMENTATIONS} PROPERTIES HEADER_FILE_ONLY TRUE)

source_group("Headers" FILES ${InputConnectors_HEADERS} ${FromStorageFile_HEADERS} )
source_group("Sources" FILES ${InputConnectors_SOURCES} ${FromStorageFile_SOURCES}  ${InputConnectors_TEMPLATE_IMPLEMENTATIONS})

# Define targets
add_library(InputConnectors STATIC
  ${InputConnectors_SOURCES}
  ${FromStorageFile_SOURCES}
  ${InputConnectors_HEADERS}
  ${FromStorageFile_HEADERS}
  ${InputConnectors_TEMPLATE_IMPLEMENTATIONS}
)
target_link_libraries(InputConnectors FileIO ${OpenSim_LIBRARIES} rtb::Filter rtb::Concurrency)

target_include_directories(InputConnectors
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${ceinms_common_INCLUDE_DIRS}
    ${FileIO_INCLUDE_DIRS}
    ${Concurrency_INCLUDE_DIRS}
	PUBLIC	
		${mcbs_INCLUDE_DIRS}
		${osimSpliner_INCLUDE_DIRS}
)

# Save path to include directory for "internal" usage
set(InputConnectors_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}"  CACHE INTERNAL "Include directories for InputConnectors library")

# Install
install(FILES ${FromStorageFile_HEADERS} ${InputConnectors_HEADERS} ${InputConnectors_TEMPLATE_IMPLEMENTATIONS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/ceinms COMPONENT Development)
install(TARGETS InputConnectors
        EXPORT CEINMSTargets
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT Applications
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT Development
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT Development
)
