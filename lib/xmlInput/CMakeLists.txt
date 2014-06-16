#INCLUDE_DIRECTORIES(~/Programs/xsd/libxsd/
#~/Programs/xerces-c-3.1.1/include/
#)

#include_directories(${XSD_INCLUDE_DIR})
#xsd_schema(subject.xml)

include_directories(${ceinms_common_INCLUDE_DIRS})
include_directories(${XERCES_INCLUDE_DIR})
include_directories(${XSD_INCLUDE_DIR})
include_directories(${Boost_INCLUDE_DIRS})

add_executable(xsdbin xsdbin.cxx)
target_link_libraries(xsdbin ${XERCES_LIBRARIES})

macro (schema_to_bin xsdfile)

  get_filename_component(xsd_name_we ${xsdfile} NAME_WE)
  add_custom_command(OUTPUT ${xsd_name_we}-schema.cxx ${xsd_name_we}-schema.hxx
                   COMMAND xsdbin ARGS #--output-dir ${CMAKE_CURRENT_SOURCE_DIR}
                   ${xsdfile}
                   MAIN_DEPENDENCY ${xsdfile}
                   DEPENDS xsdbin
                   COMMENT Generating binary representation for ${xsdfile}
                   VERBATIM)
endmacro()

schema_to_bin(${CMAKE_SOURCE_DIR}/cfg/XSD/execution.xsd)
schema_to_bin(${CMAKE_SOURCE_DIR}/cfg/XSD/calibration.xsd)
schema_to_bin(${CMAKE_SOURCE_DIR}/cfg/XSD/subject.xsd)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

set(xmlInput_SOURCES    subject.cxx
                        execution.cxx
                        emgGenerator.cxx
                        ExecutionXmlReader.cpp
                        calibration.cxx
                        CalibrationXmlReader.cpp
                        SubjectXmlReader.cpp
                        grammar-input-stream.cxx)

set(xmlInput_GENERATED_SOURCES  ${CMAKE_CURRENT_BINARY_DIR}/execution-schema.cxx
                                ${CMAKE_CURRENT_BINARY_DIR}/calibration-schema.cxx
                                ${CMAKE_CURRENT_BINARY_DIR}/subject-schema.cxx)

set(xmlInput_HEADERS    subject.hxx
                        execution.hxx
                        emgGenerator.hxx
                        ExecutionXmlReader.h
                        NMSmodelConfig.h
                        calibration.hxx
                        CalibrationXmlReader.h
                        CalibrationCfg.h
                        SubjectXmlReader.h
                        grammar-input-stream.hxx
                        validation.h)

set(xmlInput_GENERATED_HEADERS  ${CMAKE_CURRENT_BINARY_DIR}/execution-schema.hxx
                                ${CMAKE_CURRENT_BINARY_DIR}/calibration-schema.hxx
                                ${CMAKE_CURRENT_BINARY_DIR}/subject-schema.hxx)

source_group("Headers" FILES ${xmlInput_HEADERS})
source_group("Generated headers" FILES ${xmlInput_GENERATED_HEADERS})
source_group("Sources" FILES ${xmlInput_SOURCES})
source_group("Generated sources" FILES ${xmlInput_GENERATED_SOURCES})
add_library(xmlInput STATIC ${xmlInput_SOURCES}
                            ${xmlInput_HEADERS}
                            ${xmlInput_GENERATED_HEADERS}
                            ${xmlInput_GENERATED_SOURCES})

target_link_libraries(xmlInput ceinms_common ${XERCES_LIBRARIES})

set(xmlInput_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR} ${XSD_INCLUDE_DIR} CACHE STRING "Include directory for xmlInput library")
mark_as_advanced(xmlInput_INCLUDE_DIRS)

install(FILES ${xmlInput_HEADERS} ${xmlInput_GENERATED_HEADERS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}) #should add namespace
install(TARGETS xmlInput
        EXPORT CEINMSTargets
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
