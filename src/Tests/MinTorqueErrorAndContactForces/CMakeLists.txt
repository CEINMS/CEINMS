include_directories( 
	${ceinms_common_INCLUDE_DIRS}
    ${JointContactModels_INCLUDE_DIRS}
    ${OptimizationSystems_INCLUDE_DIRS}
    ${TrialData_INCLUDE_DIRS})
add_executable(Test_MinTorqueErrorAndContactForces minTorqueErrorAndContactForcesTest.cpp)
target_link_libraries(Test_MinTorqueErrorAndContactForces ceinms_common JointContactModels OptimizationSystems)

