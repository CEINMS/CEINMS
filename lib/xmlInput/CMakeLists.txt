#INCLUDE_DIRECTORIES(~/Programs/xsd/libxsd/
#~/Programs/xerces-c-3.1.1/include/
#)

#include_directories(${XSD_INCLUDE_DIR})
#xsd_schema(subject.xml)

include_directories(${ceinms_common_INCLUDE_DIRS})
include_directories(${XERCES_INCLUDE_DIR})
include_directories(${XSD_INCLUDE_DIR})
include_directories(${Boost_INCLUDE_DIRS})

#TODO: why are some files missing from these lists?
add_library(xmlInput STATIC
    subject.cxx
#  NMSmodel.cxx
  execution.cxx
  emgGenerator.cxx
  ExecutionXmlReader.cpp
)

target_link_libraries(xmlInput ${XERCES_LIBRARIES})

add_library(xmlInputCalibration STATIC
    subject.cxx
    emgGenerator.hxx
    calibration.cxx
    CalibrationXmlReader.cpp)

target_link_libraries(xmlInputCalibration ceinms_common ${XERCES_LIBRARIES})

set(xmlInput_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR} ${XSD_INCLUDE_DIR} CACHE STRING "Include directory for xmlInput library")
mark_as_advanced(xmlInput_INCLUDE_DIRS)
