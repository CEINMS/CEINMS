SET(EMGgenerator_SOURCES
  EMGgenerator/EMGgeneratorFrom10To13.cpp
  EMGgenerator/EMGgeneratorFrom16To34.cpp
  EMGgenerator/EMGgeneratorFrom6To24.cpp
)

INCLUDE_DIRECTORIES(EMGgenerator)

ADD_LIBRARY(FileIO STATIC
  DataFromFile.cpp
  EMGDataFromFile.cpp
  ${EMGgenerator_SOURCES})
