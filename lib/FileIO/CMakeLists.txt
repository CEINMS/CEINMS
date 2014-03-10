## Authors: Monica Reggiani, Claudio Pizzolato

set(EMGgenerator_SOURCES
  EMGgenerator/EMGgeneratorFrom10To13.cpp
  EMGgenerator/EMGgeneratorFrom16To34.cpp
  EMGgenerator/EMGgeneratorFrom6To24.cpp
  EMGgenerator/EMGgeneratorFromXml.cpp
)

include_directories(EMGgenerator
                    ${xmlInput_INCLUDE_DIRS}) #TODO check if need include ceinms_common

add_library(FileIO STATIC
  DataFromFile.cpp
  EMGDataFromFile.cpp
  ${EMGgenerator_SOURCES})

target_link_libraries(FileIO xmlInput)

set(FileIO_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/EMGgenerator CACHE STRING "Include directories for FileIO library")

