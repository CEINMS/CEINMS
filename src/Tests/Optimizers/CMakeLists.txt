include_directories(
	${Optimizers_INCLUDE_DIRS}
	${OptimizationSystems_INCLUDE_DIRS}
)
add_executable(Test_Optimizers optimizerTest.cpp)
#target_link_libraries(OptimizersTest Optimizers)
set_target_properties(Test_Optimizers PROPERTIES FOLDER "Tests")
add_test(NAME TestOptimizers COMMAND Test_Optimizers)