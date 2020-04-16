include_directories( 
	${OPENSIM_INCLUDE_DIRS}
	${ceinms_common_INCLUDE_DIRS}
    ${JointContactModels_INCLUDE_DIRS}
    ${OptimizationSystems_INCLUDE_DIRS}
    ${TrialData_INCLUDE_DIRS}
    ${SIMTK_INCLUDE_DIRS})
add_executable(Test_CalibrationObjectiveFunctions calibrationObjectiveFunctionsTest.cpp)
target_link_libraries(Test_CalibrationObjectiveFunctions ceinms_common JointContactModels OptimizationSystems TrialData ${OPENSIM_LIBRARIES})

