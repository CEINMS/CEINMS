#INCLUDE_DIRECTORIES(~/Programs/xsd/libxsd/
#~/Programs/xerces-c-3.1.1/include/
#)

include_directories(../Calibration)

ADD_LIBRARY(xmlInput STATIC
    subject.cxx
#  NMSmodel.cxx
  execution.cxx
  ExecutionXmlReader.cpp
)


ADD_LIBRARY(xmlInputCalibration STATIC
#  NMSmodel.cxx
  subject.cxx
  simulatedAnnealing.cxx
)

ADD_LIBRARY(xmlInputCalibrationTest STATIC
    subject.cxx
    calibration.cxx
    CalibrationXmlReader.cpp)