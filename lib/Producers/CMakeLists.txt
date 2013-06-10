SET(FromFile_SOURCES
  FromFile/EMGFromFile.cpp
  FromFile/ExternalTorqueFromFile.cpp
  FromFile/LmtMaFromFile.cpp
)

SET(FileIO_SOURCES
  ${CMAKE_SOURCE_DIR}/lib/FileIO/DataFromFile.cpp
  ${CMAKE_SOURCE_DIR}/lib/FileIO/EMGDataFromFile.cpp
  ${CMAKE_SOURCE_DIR}/lib/FileIO/EMGgenerator/EMGgeneratorFrom10To13.cpp
  ${CMAKE_SOURCE_DIR}/lib/FileIO/EMGgenerator/EMGgeneratorFrom16To34.cpp
  ${CMAKE_SOURCE_DIR}/lib/FileIO/EMGgenerator/EMGgeneratorFrom6To24.cpp
)

INCLUDE_DIRECTORIES(
  ${CMAKE_SOURCE_DIR}/lib/FileIO
  ${CMAKE_SOURCE_DIR}/lib/FileIO/EMGgenerator
  ${CMAKE_SOURCE_DIR}/lib/Producers
  ${CMAKE_SOURCE_DIR}/lib/Producers/FromFile
)

ADD_LIBRARY(Producers STATIC
  EMGFromX.cpp
  ExternalTorqueFromX.cpp
  LmtMaFromX.cpp
  Semaphore.cpp
  SyncTools.cpp
  ${FromFile_SOURCES}
  ${FileIO_SOURCES}
)
