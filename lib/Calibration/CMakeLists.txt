# Authors: Elena Ceseracciu, Monica Reggiani, Claudio Pizzolato

project(Calibration)

# List source files
set(Calibration_HEADERS TrialData.h
                        TorquesComputation.h
                        SimulatedAnnealing.h
                        ObjectiveFunctions/ObjectiveFunction.h
                        ObjectiveFunctions/SumMinObjectiveFunction_singleF.h
                        SimAnnealParameters/ParametersFromXml.h
                        SimAnnealParameters/ParametersToOptimize.h
                        SimAnnealParameters/StrengthCoefficients_ShapeFactor_C1_C2_TendonSlackLength_single.h
                        SimAnnealParameters/StrengthCoefficients_Single_ShapeFactor_C1_C2_TendonSlackLength_use_last_Dof_only.h
                        TorqueComputationModes/ComputationMode_Default.h
                        TorqueComputationModes/ComputationMode_Fast.h
                        )
set(Calibration_SOURCES )

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

source_group("Header Files" FILES ${Calibration_HEADERS})
source_group("Source Files" FILES ${Calibration_SOURCES} ${Calibration_TEMPLATE_IMPLEMENTATION})

add_library(Calibration STATIC ${Calibration_SOURCES} ${Calibration_HEADERS} ${Calibration_TEMPLATE_IMPLEMENTATION})
target_link_libraries(Calibration xmlInput FileIO)
target_include_directories( Calibration
                            PRIVATE ${NMSmodel_INCLUDE_DIRS}
                                    ${Curve_INCLUDE_DIRS}
                                    ${FileIO_INCLUDE_DIRS}
                                    ${ceinms_common_INCLUDE_DIRS})

# Save path to include directory for "internal" usage
set(Calibration_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/ObjectiveFunctions ${CMAKE_CURRENT_SOURCE_DIR}/SimAnnealParameters  ${CMAKE_CURRENT_SOURCE_DIR}/TorqueComputationModes CACHE STRING "Include directories for Calibration library")
mark_as_advanced(Calibration_INCLUDE_DIRS)

#Install
install(FILES ${Calibration_HEADERS} ${Calibration_TEMPLATE_IMPLEMENTATION} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}) #should add namespace
install(TARGETS Calibration
        EXPORT CEINMSTargets
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
