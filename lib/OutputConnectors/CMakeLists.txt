## Authors: Monica Reggiani, Claudio Pizzolato


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


include_directories(${ceinms_common_INCLUDE_DIRS}  )
include_directories(${Boost_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}  ${CMAKE_SOURCE_DIR}/lib/Concurrency ${InputConnectors_INCLUDE_DIRS})

add_library(OutputConnectors STATIC
  ${OutputConnectors_SOURCES}
  ${OutputConnectors_HEADERS}
  ${QueuesToStorageFiles_SOURCES}
  ${QueuesToStorageFiles_HEADERS}
)

set(OutputConnectors_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/QueuesToStorageFiles CACHE STRING "Include directories for InputConnectors library" FORCE)
mark_as_advanced(OutputConnectors_INCLUDE_DIRS)

install(FILES ${OutputConnectors_HEADERS} ${QueuesToStorageFiles_HEADERS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}) #should add namespace
install(TARGETS OutputConnectors
        EXPORT CEINMSTargets
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
