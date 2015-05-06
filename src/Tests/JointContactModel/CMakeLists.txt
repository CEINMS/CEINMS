include_directories( 
	${JointContactModels_INCLUDE_DIRS}
	${OPENSIM_INCLUDE_DIRS}       
    ${SIMTK_INCLUDE_DIRS}
    ${ceinms_common_INCLUDE_DIRS}      
	${xmlInput_INCLUDE_DIRS})

add_executable(Test_JointContactModel jointContactModelTest.cpp)

target_link_libraries(Test_JointContactModel  JointContactModels xmlInput optimized ${OPENSIM_LIBRARIES})

target_link_libraries(Test_JointContactModel  JointContactModels xmlInput debug ${OPENSIM_LIBRARIES_DEBUG})
