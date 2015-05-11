# Authors: Elena Ceseracciu, Monica Reggiani, Claudio Pizzolato

project(OptimizationSystems)

# List source files
set(OptimizationSystems_HEADERS ceinms/NMSmodelSystem.h
                                ceinms/ParametersInterpreter.h
                                ceinms/BatchEvaluator.h
                                ceinms/AbstractOptimizerSystem.h
                                ceinms/MinTorqueError.h
								ceinms/Result.h
                        )

set(OptimizationSystems_TEMPLATE_IMPLEMENTATION
                            ceinms/NMSmodelSystem.cpp
                            ceinms/ParametersInterpreter.cpp
							ceinms/BatchEvaluator.cpp
							)
                            
set(OptimizationSystems_SOURCES MinTorqueError.cpp Result.cpp
)

set_source_files_properties( ${OptimizationSystems_TEMPLATE_IMPLEMENTATION} PROPERTIES HEADER_FILE_ONLY TRUE)

source_group("Header Files" FILES ${OptimizationSystems_HEADERS})
source_group("Source Files" FILES ${OptimizationSystems_TEMPLATE_IMPLEMENTATION} ${OptimizationSystems_SOURCES})

add_library(OptimizationSystems ${OptimizationSystems_HEADERS} ${OptimizationSystems_TEMPLATE_IMPLEMENTATION} ${OptimizationSystems_SOURCES})
target_include_directories( OptimizationSystems
                            PRIVATE ceinms
                                    ${NMSmodel_INCLUDE_DIRS}
                                    ${Curve_INCLUDE_DIRS}
                                    ${FileIO_INCLUDE_DIRS}
                                    ${ceinms_common_INCLUDE_DIRS}
									${Optimizers_INCLUDE_DIRS}
									${TrialData_INCLUDE_DIRS})


# Save path to include directory for "internal" usage
set(OptimizationSystems_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}"
        CACHE INTERNAL "Include directories for OptimizationSystems library")

#Install
#install(FILES ${OptimizationSystems_HEADERS} ${OptimizationSystems_TEMPLATE_IMPLEMENTATION} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}) #should add namespace
#install(TARGETS OptimizationSystems
#        EXPORT CEINMSTargets
#        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
#        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
#        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
