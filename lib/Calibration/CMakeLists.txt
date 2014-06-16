# Authors: Elena Ceseracciu, Monica Reggiani, Claudio Pizzolato

project(Calibration)
include_directories(
  ${xmlInput_INCLUDE_DIRS}
  ${NMSmodel_INCLUDE_DIRS}
  ${CMAKE_SOURCE_DIR}/lib/Curve
  ${FileIO_INCLUDE_DIRS}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${ceinms_common_INCLUDE_DIRS}
)

include_directories(${Boost_INCLUDE_DIRS})

set(Calibration_HEADERS TrialData.h
                        TorquesComputation.h
                        SimulatedAnnealing.h
                        InputDataInterpreter.h
                        ObjectiveFunctions/ObjectiveFunction.h
                        ObjectiveFunctions/SumMinObjectiveFunction_singleF.h
                        SimAnnealParameters/ParametersFromXml.h
                        SimAnnealParameters/ParametersToOptimize.h
                        SimAnnealParameters/StrengthCoefficients_ShapeFactor_C1_C2_TendonSlackLength_single.h
                        SimAnnealParameters/StrengthCoefficients_Single_ShapeFactor_C1_C2_TendonSlackLength_use_last_Dof_only.h
                        TorqueComputationModes/ComputationMode_Default.h
                        TorqueComputationModes/ComputationMode_Fast.h
                        )
set(Calibration_SOURCES InputDataInterpreter.cpp)

set(Calibration_TEMPLATE_IMPLEMENTATION
                            TrialData.cpp
                            TorquesComputation.cpp
                            SimulatedAnnealing.cpp
                            ObjectiveFunctions/SumMinObjectiveFunction_singleF.cpp
                            SimAnnealParameters/ParametersFromXml.cpp
                            SimAnnealParameters/StrengthCoefficients_ShapeFactor_C1_C2_TendonSlackLength_single.cpp
                            SimAnnealParameters/StrengthCoefficients_Single_ShapeFactor_C1_C2_TendonSlackLength_use_last_Dof_only.cpp
                            TorqueComputationModes/ComputationMode_Default.cpp
                            TorqueComputationModes/ComputationMode_Fast.cpp)

set_source_files_properties( ${Calibration_TEMPLATE_IMPLEMENTATION} PROPERTIES HEADER_FILE_ONLY TRUE)

if(OPENSIM_FOUND)
  include_directories(${OPENSIM_INCLUDE_DIRS} ${SIMTK_INCLUDE_DIRS})
  list(APPEND Calibration_HEADERS InputDataInterpreterOsim.h )
  list(APPEND Calibration_SOURCES InputDataInterpreterOsim.cpp )
endif()

source_group("Header Files" FILES ${Calibration_HEADERS})
source_group("Source Files" FILES ${Calibration_SOURCES} ${Calibration_TEMPLATE_IMPLEMENTATION})

add_library(Calibration STATIC ${Calibration_SOURCES} ${Calibration_HEADERS} ${Calibration_TEMPLATE_IMPLEMENTATION})
target_link_libraries(Calibration xmlInput FileIO)

if(OPENSIM_FOUND)
  target_link_libraries(Calibration ${OPENSIM_LIBRARIES}) #should find out which ones are actually needed
endif()

set(Calibration_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/ObjectiveFunctions ${CMAKE_CURRENT_SOURCE_DIR}/SimAnnealParameters  ${CMAKE_CURRENT_SOURCE_DIR}/TorqueComputationModes CACHE STRING "Include directories for Calibration library")

install(FILES ${Calibration_HEADERS} ${Calibration_TEMPLATE_IMPLEMENTATION} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}) #should add namespace
install(TARGETS Calibration
        EXPORT CEINMSTargets
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
