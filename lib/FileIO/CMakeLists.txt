## Authors: Monica Reggiani, Claudio Pizzolato

set(EMGgenerator_SOURCES
  EMGgenerator/EMGgeneratorFrom10To13.cpp
  EMGgenerator/EMGgeneratorFrom16To34.cpp
  EMGgenerator/EMGgeneratorFrom6To24.cpp
  EMGgenerator/EMGgeneratorFromXml.cpp
  DataFromFile.cpp
  EMGDataFromFile.cpp
)

set(EMGgenerator_HEADERS
  EMGgenerator/EMGgeneratorFrom10To13.h
  EMGgenerator/EMGgeneratorFrom16To34.h
  EMGgenerator/EMGgeneratorFrom6To24.h
  EMGgenerator/EMGgeneratorFromXml.h
  DataFromFile.h
  EMGDataFromFile.h
  policyTemplates.h
)
include_directories(${Boost_INCLUDE_DIRS})
include_directories(EMGgenerator
                    ${xmlInput_INCLUDE_DIRS}) #TODO check if need include ceinms_common

source_group("Headers" FILES ${EMGgenerator_HEADERS})
source_group("Sources" FILES ${EMGgenerator_SOURCES})
add_library(FileIO STATIC ${EMGgenerator_SOURCES} ${EMGgenerator_HEADERS})

target_link_libraries(FileIO xmlInput)

set(FileIO_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/EMGgenerator CACHE STRING "Include directories for FileIO library")

