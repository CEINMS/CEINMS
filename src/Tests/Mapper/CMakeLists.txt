include_directories( 
	${ceinms_common_INCLUDE_DIRS}
)

add_executable(Test_Mapper mapperTest.cpp)
target_link_libraries(Test_Mapper ceinms_common)

set_target_properties(Test_Mapper PROPERTIES FOLDER "Tests")