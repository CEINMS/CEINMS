#INCLUDE_DIRECTORIES(~/Programs/xsd/libxsd/
#~/Programs/xerces-c-3.1.1/include/
#)

#include_directories(${XSD_INCLUDE_DIR})
#xsd_schema(subject.xml)

## Create xsdbin executable
add_executable(xsdbin xsdbin.cxx)
target_link_libraries(xsdbin ${XERCES_LIBRARIES})
target_include_directories(xsdbin PRIVATE ${XERCES_INCLUDE_DIR})

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

# Use the xsdbin executable to create binary representations of xsd schemas
# so that they can be used for validation
schema_to_bin(${CMAKE_SOURCE_DIR}/cfg/XSD/execution.xsd)
schema_to_bin(${CMAKE_SOURCE_DIR}/cfg/XSD/calibration.xsd)
schema_to_bin(${CMAKE_SOURCE_DIR}/cfg/XSD/subject.xsd)
schema_to_bin(${CMAKE_SOURCE_DIR}/cfg/XSD/emgGenerator.xsd)
schema_to_bin(${CMAKE_SOURCE_DIR}/cfg/XSD/inputData.xsd)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

# Generate c++ code for xsd schema files
xsd_schema(subject_GENERATED_FILES ${CMAKE_SOURCE_DIR}/cfg/XSD/subject.xsd --generate-serialization)
xsd_schema(emgGenerator_GENERATED_FILES ${CMAKE_SOURCE_DIR}/cfg/XSD/emgGenerator.xsd --generate-serialization)
xsd_schema(calibration_GENERATED_FILES ${CMAKE_SOURCE_DIR}/cfg/XSD/calibration.xsd --generate-serialization --namespace-map =CalibrationXsd)
xsd_schema(execution_GENERATED_FILES ${CMAKE_SOURCE_DIR}/cfg/XSD/execution.xsd --generate-serialization)
xsd_schema(inputData_GENERATED_FILES ${CMAKE_SOURCE_DIR}/cfg/XSD/inputData.xsd --generate-serialization)

set(xmlInput_SOURCES    ExecutionXmlReader.cpp
                        CalibrationXmlReader.cpp
                        SubjectXmlReader.cpp
                        grammar-input-stream.cxx
                        EmgGeneratorXmlReader.cpp
                        InputDataXmlReader.cpp)

set(xmlInput_GENERATED_SOURCES  ${CMAKE_CURRENT_BINARY_DIR}/execution-schema.cxx
                                ${CMAKE_CURRENT_BINARY_DIR}/calibration-schema.cxx
                                ${CMAKE_CURRENT_BINARY_DIR}/subject-schema.cxx
                                ${CMAKE_CURRENT_BINARY_DIR}/emgGenerator-schema.cxx
                                ${CMAKE_CURRENT_BINARY_DIR}/inputData-schema.cxx)

set(xmlInput_HEADERS    ExecutionXmlReader.h
                        NMSmodelConfig.h
                        CalibrationXmlReader.h
                        CalibrationCfg.h
                        SubjectXmlReader.h
                        EmgGeneratorXmlReader.h
                        InputDataXmlReader.h)

set(xmlInput_IMPL_HEADERS   impl/grammar-input-stream.hxx
                            impl/validation.h)

set(xmlInput_GENERATED_HEADERS  ${CMAKE_CURRENT_BINARY_DIR}/execution-schema.hxx
                                ${CMAKE_CURRENT_BINARY_DIR}/calibration-schema.hxx
                                ${CMAKE_CURRENT_BINARY_DIR}/subject-schema.hxx
                                ${CMAKE_CURRENT_BINARY_DIR}/emgGenerator-schema.hxx
                                ${CMAKE_CURRENT_BINARY_DIR}/inputData-schema.hxx)

source_group("Headers" FILES ${xmlInput_HEADERS})
source_group("Private headers" FILES ${xmlInput_IMPL_HEADERS})
source_group("Generated headers" FILES ${xmlInput_GENERATED_HEADERS})
source_group("Sources" FILES ${xmlInput_SOURCES})
source_group("Generated sources" FILES ${xmlInput_GENERATED_SOURCES})
#TODO: how do I add the outputs of xsd_schema ?


add_library(xmlInput STATIC ${xmlInput_SOURCES}
                            ${xmlInput_HEADERS}
                            ${xmlInput_IMPL_HEADERS}
                            ${xmlInput_GENERATED_HEADERS}
                            ${xmlInput_GENERATED_SOURCES}
                            ${subject_GENERATED_FILES}
                            ${emgGenerator_GENERATED_FILES}
                            ${calibration_GENERATED_FILES}
                            ${execution_GENERATED_FILES}
                            ${inputData_GENERATED_FILES}
                            )

target_link_libraries(xmlInput ceinms_common ${XERCES_LIBRARIES})
target_include_directories(xmlInput PRIVATE ${ceinms_common_INCLUDE_DIRS} ${XERCES_INCLUDE_DIR} impl PUBLIC ${XSD_INCLUDE_DIR})

set(xmlInput_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${XSD_INCLUDE_DIR} CACHE INTERNAL "Include directory for xmlInput library")
mark_as_advanced(xmlInput_INCLUDE_DIRS)

install(FILES ${xmlInput_HEADERS} ${xmlInput_GENERATED_HEADERS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}) #should add namespace
install(TARGETS xmlInput
        EXPORT CEINMSTargets
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        # INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR} # only available from CMake 2.8.12
        )
