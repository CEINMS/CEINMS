SET(EMGgenerator_SOURCES
  EMGgenerator/EMGgeneratorFrom10To13.cpp
  EMGgenerator/EMGgeneratorFrom16To34.cpp
  EMGgenerator/EMGgeneratorFrom6To24.cpp
  EMGgenerator/EMGgeneratorFromXml.cpp
  ${CMAKE_SOURCE_DIR}/lib/xmlInput/emgGenerator.cxx
)

INCLUDE_DIRECTORIES(EMGgenerator
                    ${CMAKE_SOURCE_DIR}/lib/xmlInput)

ADD_LIBRARY(FileIO STATIC
  DataFromFile.cpp
  EMGDataFromFile.cpp
  ${EMGgenerator_SOURCES})
