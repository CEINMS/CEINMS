## Authors: Monica Reggiani, Claudio Pizzolato, Elena Ceseracciu

# List source files
set(FileIO_SOURCES
  ExcitationGeneratorFromXml.cpp
  DataFromFile.cpp
  DataFromStorageFile.cpp
  EMGDataFromFile.cpp
)

set(FileIO_HEADERS
  ExcitationGeneratorFromXml.h
  DataFromFile.h
  DataFromStorageFile.h
  EMGDataFromFile.h
  policyTemplates.h
)

source_group("Headers" FILES ${FileIO_HEADERS})
source_group("Sources" FILES ${FileIO_SOURCES})

# Define target
add_library(FileIO STATIC ${FileIO_SOURCES} ${FileIO_HEADERS})
target_link_libraries(FileIO xmlInput)
target_include_directories(FileIO PRIVATE EMGgenerator ${xmlInput_INCLUDE_DIRS} PUBLIC ${Boost_INCLUDE_DIRS})

# Save path to include directory for "internal" usage
set(FileIO_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR} ${xmlInput_INCLUDE_DIRS} CACHE STRING "Include directories for FileIO library")
mark_as_advanced(FileIO_INCLUDE_DIRS)

# Install
install(FILES ${FileIO_HEADERS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}) #should add namespace
install(TARGETS FileIO
        EXPORT CEINMSTargets
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
