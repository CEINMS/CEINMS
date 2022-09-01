include_directories( 
	${Curve_INCLUDE_DIRS}
)

add_executable(Test_Curve curveTest.cpp)

set_target_properties(Test_Curve PROPERTIES FOLDER "Tests")