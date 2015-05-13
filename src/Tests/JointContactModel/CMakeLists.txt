include_directories( 
	${JointContactModels_INCLUDE_DIRS}
	${OPENSIM_INCLUDE_DIRS}       
    ${SIMTK_INCLUDE_DIRS}
    ${ceinms_common_INCLUDE_DIRS}      
	${xmlInput_INCLUDE_DIRS}
	${OutputConnectors_INCLUDE_DIRS}
	${InputConnectors_INCLUDE_DIRS}
	${Concurrency_INCLUDE_DIRS}
	${NMSmodel_INCLUDE_DIRS}
	${Curve_INCLUDE_DIRS})

#add_executable(Test_JointContactModel jointContactModelTest.cpp)

#target_link_libraries(Test_JointContactModel  JointContactModels xmlInput optimized ${OPENSIM_LIBRARIES})

#target_link_libraries(Test_JointContactModel  JointContactModels xmlInput debug ${OPENSIM_LIBRARIES_DEBUG})


add_executable(Test_JointContactModelQueues jointContactModelQueueTest.cpp)
target_link_libraries(Test_JointContactModelQueues  JointContactModels OutputConnectors InputConnectors xmlInput Concurrency NMSmodel optimized ${OPENSIM_LIBRARIES})
target_link_libraries(Test_JointContactModelQueues  JointContactModels OutputConnectors InputConnectors xmlInput Concurrency NMSmodel debug ${OPENSIM_LIBRARIES_DEBUG})
