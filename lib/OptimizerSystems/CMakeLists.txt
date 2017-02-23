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
                            ceinms/BatchEvaluator.cpp)
                            
set(OptimizationSystems_SOURCES MinTorqueError.cpp Result.cpp
)

set_source_files_properties( ${OptimizationSystems_TEMPLATE_IMPLEMENTATION} PROPERTIES HEADER_FILE_ONLY TRUE)

source_group("Header Files" FILES ${OptimizationSystems_HEADERS})
source_group("Source Files" FILES ${OptimizationSystems_TEMPLATE_IMPLEMENTATION} ${OptimizationSystems_SOURCES})

add_library(OptimizationSystems ${OptimizationSystems_HEADERS} ${OptimizationSystems_TEMPLATE_IMPLEMENTATION} ${OptimizationSystems_SOURCES})
target_link_libraries(OptimizationSystems NMSmodel Curve FileIO ceinms_common Optimizers TrialData)
target_include_directories( OptimizationSystems PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
                                              $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)


#Install
install(FILES ${OptimizationSystems_HEADERS} ${OptimizationSystems_TEMPLATE_IMPLEMENTATION} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/ceinms COMPONENT Development)
install(TARGETS OptimizationSystems
        EXPORT CEINMSTargets
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT Applications
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT Development
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT Development)
