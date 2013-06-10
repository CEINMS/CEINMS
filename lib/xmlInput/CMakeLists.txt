#INCLUDE_DIRECTORIES(~/Programs/xsd/libxsd/
#~/Programs/xerces-c-3.1.1/include/
#)

ADD_LIBRARY(xmlInput STATIC
  NMSmodel.cxx
  execution.cxx
  ExecutionXmlReader.cpp
)


ADD_LIBRARY(xmlInputCalibration STATIC
  NMSmodel.cxx
  simulatedAnnealing.cxx
)
