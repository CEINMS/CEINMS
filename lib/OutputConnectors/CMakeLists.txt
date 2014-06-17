## Authors: Monica Reggiani, Claudio Pizzolato, Elena Ceseracciu

# List source files
set(OutputConnectors_SOURCES
    OutputQueues.cpp
    LoggerOnQueues.cpp
)
set(OutputConnectors_HEADERS 
    OutputQueues.h
    LoggerOnQueues.h
)

set(QueuesToStorageFiles_SOURCES
  QueuesToStorageFiles/QueuesToStorageFiles.cpp
)

set(QueuesToStorageFiles_HEADERS
  QueuesToStorageFiles/QueuesToStorageFiles.h
)

# Define target
add_library(OutputConnectors STATIC
  ${OutputConnectors_SOURCES}
  ${OutputConnectors_HEADERS}
  ${QueuesToStorageFiles_SOURCES}
  ${QueuesToStorageFiles_HEADERS}
)
target_link_libraries(OutputConnectors ${Boost_FILESYSTEM_LIBRARY})
target_include_directories(OutputConnectors
                                PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
                                        ${ceinms_common_INCLUDE_DIRS}
                                        ${InputConnectors_INCLUDE_DIRS}
                                        ${Concurrency_INCLUDE_DIRS}
                                PUBLIC ${Boost_INCLUDE_DIRS})

# Save path to include directory for "internal" usage
set(OutputConnectors_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/QueuesToStorageFiles CACHE STRING "Include directories for OutputConnectors library" FORCE)
mark_as_advanced(OutputConnectors_INCLUDE_DIRS)

# Install
install(FILES ${OutputConnectors_HEADERS} ${QueuesToStorageFiles_HEADERS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}) #should add namespace
install(TARGETS OutputConnectors
        EXPORT CEINMSTargets
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
