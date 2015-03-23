include_directories( 
	${TrialData_INCLUDE_DIRS})

add_executable(Test_DataTable dataTableTest.cpp)
TARGET_LINK_LIBRARIES(Test_DataTable       
	  TrialData)

