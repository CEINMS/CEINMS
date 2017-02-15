## Authors: Monica Reggiani, Claudio Pizzolato, Elena Ceseracciu

# List source files
set(FromStorageFile_SOURCES
  FromStorageFile/EMGFromFile.cpp
  FromStorageFile/ExternalTorquesFromStorageFile.cpp
  FromStorageFile/LmtMaFromStorageFile.cpp
  FromStorageFile/Utilities.cpp
)
set(FromStorageFile_HEADERS
  ceinms/EMGFromFile.h
  ceinms/ExternalTorquesFromStorageFile.h
  ceinms/LmtMaFromStorageFile.h
  ceinms/Utilities.h
)

set(InputConnectors_SOURCES
  EMGFromX.cpp
  ExternalTorquesFromX.cpp
  LmtMaFromX.cpp
  InputConnectors.cpp
)
set(InputConnectors_HEADERS
  ceinms/EMGFromX.h
  ceinms/ExternalTorquesFromX.h
  ceinms/LmtMaFromX.h
  ceinms/InputConnectors.h
)

source_group("Headers" FILES ${InputConnectors_HEADERS} ${FromStorageFile_HEADERS} )
source_group("Sources" FILES ${InputConnectors_SOURCES} ${FromStorageFile_SOURCES} )

# Define targets
add_library(InputConnectors STATIC
  ${InputConnectors_SOURCES}
  ${FromStorageFile_SOURCES}
  ${InputConnectors_HEADERS}
  ${FromStorageFile_HEADERS}
)
target_link_libraries(InputConnectors FileIO rtb::Concurrency ${Boost_FILESYSTEM_LIBRARY})
target_include_directories(InputConnectors
                                PRIVATE     ${CMAKE_CURRENT_SOURCE_DIR}
                                            ${ceinms_common_INCLUDE_DIRS}
                                            ${FileIO_INCLUDE_DIRS}
                                            ${Boost_INCLUDE_DIRS})

# Save path to include directory for "internal" usage
set(InputConnectors_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}"  CACHE INTERNAL "Include directories for InputConnectors library")


# Install
install(FILES ${FromStorageFile_HEADERS} ${InputConnectors_HEADERS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/ceinms COMPONENT Development)
install(TARGETS InputConnectors
        EXPORT CEINMSTargets
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT Applications
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT Development
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT Development)
