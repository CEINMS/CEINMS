include_directories( 
	${ceinms_common_INCLUDE_DIRS}
    ${JointContactModels_INCLUDE_DIRS})
add_executable(Test_ContactModelKneeBalance contactModelKneeBalanceTest.cpp)
target_link_libraries(Test_ContactModelKneeBalance ceinms_common JointContactModels)

