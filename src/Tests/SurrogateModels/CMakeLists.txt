include_directories( 
	${ceinms_common_INCLUDE_DIRS}
	${Curve_INCLUDE_DIRS}
)

add_definitions(/DBASE_DIR="${CMAKE_CURRENT_LIST_DIR}")

add_executable(Test_SurrogateModels surrogateModelsTest.cpp)
target_link_libraries(Test_SurrogateModels ${OpenSim_LIBRARIES})
