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
include_directories(${CMAKE_CURRENT_SOURCE_DIR}  ${CMAKE_SOURCE_DIR}/lib/Concurrency ${CMAKE_SOURCE_DIR}/lib/InputConnectors)

add_library(OutputConnectors STATIC
  ${OutputConnectors_SOURCES}
  ${OutputConnectors_HEADERS}
  ${QueuesToStorageFiles_SOURCES}
  ${QueuesToStorageFiles_HEADERS}
)

set(OutputConnectors_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/QueuesToStorageFiles CACHE STRING "Include directories for InputConnectors library" FORCE)
