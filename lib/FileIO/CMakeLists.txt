## Authors: Monica Reggiani, Claudio Pizzolato, Elena Ceseracciu

# List source files
set(EMGgenerator_SOURCES
  EMGgenerator/EMGgeneratorFrom10To13.cpp
  EMGgenerator/EMGgeneratorFrom16To34.cpp
  EMGgenerator/EMGgeneratorFrom6To24.cpp
  EMGgenerator/EMGgeneratorFromXml.cpp
  DataFromFile.cpp
  DataFromStorageFile.cpp
  EMGDataFromFile.cpp
)

set(EMGgenerator_HEADERS 
  EMGgenerator/EMGgeneratorFrom10To13.h
  EMGgenerator/EMGgeneratorFrom16To34.h
  EMGgenerator/EMGgeneratorFrom6To24.h
  EMGgenerator/EMGgeneratorFromXml.h
  DataFromFile.h
  DataFromStorageFile.h
  EMGDataFromFile.h
  policyTemplates.h
)

source_group("Headers" FILES ${EMGgenerator_HEADERS})
source_group("Sources" FILES ${EMGgenerator_SOURCES})

# Define target
add_library(FileIO STATIC ${EMGgenerator_SOURCES} ${EMGgenerator_HEADERS})
target_link_libraries(FileIO xmlInput)
target_include_directories(FileIO PRIVATE EMGgenerator ${xmlInput_INCLUDE_DIRS} PUBLIC ${Boost_INCLUDE_DIRS})

# Save path to include directory for "internal" usage
set(FileIO_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/EMGgenerator ${xmlInput_INCLUDE_DIRS} CACHE STRING "Include directories for FileIO library")
mark_as_advanced(FileIO_INCLUDE_DIRS)

# Install
install(FILES ${EMGgenerator_HEADERS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}) #should add namespace
install(TARGETS FileIO
        EXPORT CEINMSTargets
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
