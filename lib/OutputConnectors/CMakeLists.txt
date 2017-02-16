## Authors: Monica Reggiani, Claudio Pizzolato, Elena Ceseracciu

# List source files
set(OutputConnectors_SOURCES
    OutputConnectors.cpp
    LoggerOnQueues.cpp
    QueuesToTrialData.cpp
)
set(OutputConnectors_HEADERS 
    ceinms/OutputConnectors.h
    ceinms/LoggerOnQueues.h
    ceinms/QueuesToTrialData.h
)

set(QueuesToStorageFiles_SOURCES
  QueuesToStorageFiles/QueuesToStorageFiles.cpp
)

set(QueuesToStorageFiles_HEADERS
    ceinms/QueuesToStorageFiles.h
)

# Define target
add_library(OutputConnectors STATIC
  ${OutputConnectors_SOURCES}
  ${OutputConnectors_HEADERS}
  ${QueuesToStorageFiles_SOURCES}
  ${QueuesToStorageFiles_HEADERS}
)
target_link_libraries(OutputConnectors rtb::Concurrency ${Boost_FILESYSTEM_LIBRARY} ceinms_common InputConnectors TrialData)
target_include_directories(OutputConnectors PUBLIC ${Boost_INCLUDE_DIRS}
                                                   $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
                                                   $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

# Install
install(FILES ${OutputConnectors_HEADERS} ${QueuesToStorageFiles_HEADERS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/ceinms COMPONENT Development)
install(TARGETS OutputConnectors
        EXPORT CEINMSTargets
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT Applications
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT Development
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT Development)
