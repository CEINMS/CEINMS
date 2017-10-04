# Authors: Elena Ceseracciu, Monica Reggiani, Claudio Pizzolato

project(OptimizationSystems)

# List source files
set(OptimizationSystems_HEADERS ceinms/NMSmodelSystem.h
                                ceinms/ParametersInterpreter.h
                                ceinms/BatchEvaluator.h
								ceinms/BatchOpenSimEvaluator.h
                                ceinms/BatchContactModelEvaluator.h
                                ceinms/AbstractOptimizerSystem.h
                                ceinms/MinTorqueError.h
								ceinms/Result.h
                        #        ceinms/MinTorqueErrorAndContactForces.h
                                ceinms/Barrier.h
                                ceinms/BarrierFunction.h
                                ceinms/SumKneeContactForces.h
                      #          ceinms/SumGenericContactForces.h
								ceinms/ComposedObjectiveFunction.h
								ceinms/ObjectiveFunctionBase.h
                        )

set(OptimizationSystems_TEMPLATE_IMPLEMENTATION
                            ceinms/NMSmodelSystem.cpp
                            ceinms/ParametersInterpreter.cpp
							ceinms/BatchEvaluator.cpp
							)
                            
set(OptimizationSystems_SOURCES 
    MinTorqueError.cpp 
    Result.cpp 
    SumKneeContactForces.cpp 
#    MinTorqueErrorAndContactForces.cpp
#    SumGenericContactForces.cpp
    BarrierFunction.cpp
	BatchOpenSimEvaluator.cpp
    BatchContactModelEvaluator.cpp
    ComposedObjectiveFunction.cpp
)

set(OptimizationSystems_Hybrid_HEADERS 
	ceinms/Hybrid/EmgInformedSystem.h
	ceinms/Hybrid/TrackTorqueAdjustAndPredictExcitations.h
    ceinms/Hybrid/MuscleExcitationParameterInterpreter.h
	ceinms/Hybrid/NMSmodelFrameOptimisation.h)
set(OptimizationSystems_Hybrid_TEMPLATE_IMPLEMENTATION 
	ceinms/Hybrid/EmgInformedSystem.cpp
    ceinms/Hybrid/NMSmodelFrameOptimisation.cpp
	ceinms/Hybrid/MuscleExcitationParameterInterpreter.cpp)
set(OptimizationSystems_Hybrid_SOURCES 
	Hybrid/TrackTorqueAdjustAndPredictExcitations.cpp
    
	)

set_source_files_properties( ${OptimizationSystems_TEMPLATE_IMPLEMENTATION} PROPERTIES HEADER_FILE_ONLY TRUE)
set_source_files_properties( ${OptimizationSystems_Hybrid_TEMPLATE_IMPLEMENTATION} PROPERTIES HEADER_FILE_ONLY TRUE)

source_group("Header Files" FILES ${OptimizationSystems_HEADERS})
source_group("Source Files" FILES ${OptimizationSystems_TEMPLATE_IMPLEMENTATION} ${OptimizationSystems_SOURCES})

source_group("Hybrid/Header Files" FILES ${OptimizationSystems_Hybrid_HEADERS})
source_group("Hybrid/Source Files" FILES ${OptimizationSystems_Hybrid_TEMPLATE_IMPLEMENTATION} ${OptimizationSystems_Hybrid_SOURCES})

add_library(OptimizationSystems 
${OptimizationSystems_HEADERS} 
${OptimizationSystems_TEMPLATE_IMPLEMENTATION} 
${OptimizationSystems_SOURCES}
${OptimizationSystems_Hybrid_HEADERS} 
${OptimizationSystems_Hybrid_TEMPLATE_IMPLEMENTATION} 
${OptimizationSystems_Hybrid_SOURCES}

)
target_include_directories( OptimizationSystems
                            PRIVATE ceinms
                                    ${OPENSIM_INCLUDE_DIRS}       
                                    ${SIMTK_INCLUDE_DIRS}
                                    ${CMAKE_CURRENT_SOURCE_DIR}
                                    ${NMSmodel_INCLUDE_DIRS}
                                    ${Curve_INCLUDE_DIRS}
                                    ${FileIO_INCLUDE_DIRS}
                                    ${ceinms_common_INCLUDE_DIRS}
									${Optimizers_INCLUDE_DIRS}
									${TrialData_INCLUDE_DIRS}
                                    ${JointContactModels_INCLUDE_DIRS})

target_link_libraries(OptimizationSystems ceinms_common JointContactModels  ${OPENSIM_LIBRARIES})

# Save path to include directory for "internal" usage
set(OptimizationSystems_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}"
        CACHE INTERNAL "Include directories for OptimizationSystems library")

#Install
install(FILES ${OptimizationSystems_HEADERS} ${OptimizationSystems_TEMPLATE_IMPLEMENTATION} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/ceinms COMPONENT Development)

install(FILES ${OptimizationSystems_Hybrid_HEADERS} ${OptimizationSystems_Hybrid_TEMPLATE_IMPLEMENTATION} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/ceinms/Hybrid COMPONENT Development)


install(TARGETS OptimizationSystems
        EXPORT CEINMSTargets
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT Applications
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT Development
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT Development)
