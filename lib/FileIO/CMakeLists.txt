## Authors: Monica Reggiani, Claudio Pizzolato, Elena Ceseracciu

# List source files
set(FileIO_SOURCES
  ExcitationGeneratorFromXml.cpp
  DataFromFile.cpp
  DataFromStorageFile.cpp
  EMGDataFromFile.cpp
)

set(FileIO_HEADERS
  ceinms/ExcitationGeneratorFromXml.h
  ceinms/DataFromFile.h
  ceinms/DataFromStorageFile.h
  ceinms/EMGDataFromFile.h
  ceinms/policyTemplates.h
)

source_group("Headers" FILES ${FileIO_HEADERS})
source_group("Sources" FILES ${FileIO_SOURCES})

# Define target
add_library(FileIO STATIC ${FileIO_SOURCES} ${FileIO_HEADERS})
target_link_libraries(FileIO xmlInput)
target_include_directories(FileIO PRIVATE ceinms ${xmlInput_INCLUDE_DIRS} PUBLIC ${Boost_INCLUDE_DIRS})

# Save path to include directory for "internal" usage
set(FileIO_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}" ${xmlInput_INCLUDE_DIRS} CACHE INTERNAL "Include directories for FileIO library")

# Install
install(FILES ${FileIO_HEADERS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/ceinms COMPONENT Development)
install(TARGETS FileIO
        EXPORT CEINMSTargets
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT Applications
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT Development
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT Development)
