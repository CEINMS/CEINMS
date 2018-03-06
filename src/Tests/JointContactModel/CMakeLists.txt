include_directories( 
	${JointContactModels_INCLUDE_DIRS}
	${OPENSIMSIMBODY_INCLUDE_DIRS}
    ${ceinms_common_INCLUDE_DIRS}      
	${xmlInput_INCLUDE_DIRS}
	${OutputConnectors_INCLUDE_DIRS}
	${InputConnectors_INCLUDE_DIRS}
	${Concurrency_INCLUDE_DIRS}
	${NMSmodel_INCLUDE_DIRS}
	${Curve_INCLUDE_DIRS}
	${FileIO_INCLUDE_DIRS}
    ${TrialData_INCLUDE_DIRS}
    ${OptimizationSystems_INCLUDE_DIRS}   )
add_definitions(/DBASE_DIR="${CMAKE_CURRENT_LIST_DIR}")

add_executable(Test_JointContactModelFastExecution jointContactModelFastExecutionTest.cpp)
target_link_libraries(Test_JointContactModelFastExecution  JointContactModels OptimizationSystems OutputConnectors InputConnectors  xmlInput TrialData NMSmodel ${OPENSIMSIMBODY_LIBRARIES})
#target_link_libraries(Test_JointContactModelFastExecution  JointContactModels OptimizationSystems OutputConnectors InputConnectors  xmlInput TrialData NMSmodel debug ${OPENSIM_LIBRARIES_DEBUG})


add_executable(Test_JointContactModelQueues jointContactModelQueueTest.cpp)
target_link_libraries(Test_JointContactModelQueues  JointContactModels OutputConnectors InputConnectors xmlInput Concurrency NMSmodel  ${OPENSIMSIMBODY_LIBRARIES})
#target_link_libraries(Test_JointContactModelQueues  JointContactModels OutputConnectors InputConnectors xmlInput Concurrency NMSmodel debug ${OPENSIM_LIBRARIES_DEBUG})
