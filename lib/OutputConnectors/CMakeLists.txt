## Authors: Monica Reggiani, Claudio Pizzolato


set(OutputConnectors_SOURCES
    OutputQueues.cpp
    LoggerOnQueues.cpp
)

set(OutputConnectors_HEADERS 
    OutputQueues.h
    LoggerOnQueues.h
)

include_directories(${Boost_INCLUDE_DIRS})
include_directories(${ceinms_common_INCLUDE_DIRS} ${CMAKE_SOURCE_DIR}/lib/Concurrency)


add_library(OutputConnectors STATIC
  ${OutputConnectors_SOURCES}
  ${OutputConnectors_HEADERS}
)

set(OutputConnectors_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}  CACHE STRING "Include directories for InputConnectors library" FORCE)
