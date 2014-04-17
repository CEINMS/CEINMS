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
set(xmlInput_SOURCES  subject.cxx execution.cxx emgGenerator.cxx ExecutionXmlReader.cpp)
set(xmlInput_HEADERS  subject.hxx execution.hxx emgGenerator.hxx ExecutionXmlReader.h NMSmodelConfig.h)

source_group("Headers" FILES ${xmlInput_HEADERS})
source_group("Sources" FILES ${xmlInput_SOURCES})
add_library(xmlInput STATIC ${xmlInput_SOURCES} ${xmlInput_HEADERS}
#  NMSmodel.cxx
)

target_link_libraries(xmlInput ${XERCES_LIBRARIES})


set(xmlInputCalibration_SOURCES  subject.cxx calibration.cxx emgGenerator.cxx CalibrationXmlReader.cpp)
set(xmlInputCalibration_HEADERS  subject.hxx calibration.hxx emgGenerator.hxx CalibrationXmlReader.h CalibrationCfg.h NMSmodelConfig.h)

source_group("Headers" FILES ${xmlInputCalibration_HEADERS})
source_group("Sources" FILES ${xmlInputCalibration_SOURCES})

add_library(xmlInputCalibration STATIC ${xmlInputCalibration_SOURCES} ${xmlInputCalibration_HEADERS})

target_link_libraries(xmlInputCalibration ceinms_common ${XERCES_LIBRARIES})

set(xmlInput_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR} ${XSD_INCLUDE_DIR} CACHE STRING "Include directory for xmlInput library")
mark_as_advanced(xmlInput_INCLUDE_DIRS)
