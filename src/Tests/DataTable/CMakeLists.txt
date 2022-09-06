include_directories(${TrialData_INCLUDE_DIRS})
add_executable(Test_DataTable dataTableTest.cpp)
set_target_properties(Test_DataTable PROPERTIES FOLDER "Tests")
add_test(NAME TestDataTable COMMAND Test_DataTable)