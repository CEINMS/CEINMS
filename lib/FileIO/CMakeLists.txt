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
add_library(FileIO ${FileIO_SOURCES} ${FileIO_HEADERS})
target_link_libraries(FileIO xmlInput)
target_include_directories(FileIO PUBLIC ${Boost_INCLUDE_DIRS}
                                        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
                                        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)


# Install
install(FILES ${FileIO_HEADERS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/ceinms COMPONENT Development)
install(TARGETS FileIO
        EXPORT CEINMSTargets
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT Applications
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT Development
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT Development)
