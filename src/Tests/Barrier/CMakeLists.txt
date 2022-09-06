add_executable(Test_Barrier barrierTest.cpp)
set_target_properties(Test_Barrier PROPERTIES FOLDER "Tests")
add_test(NAME TestBarrier COMMAND Test_Barrier)