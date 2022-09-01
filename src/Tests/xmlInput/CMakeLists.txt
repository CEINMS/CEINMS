include_directories(
	${xmlInput_INCLUDE_DIRS}
	${ceinms_common_INCLUDE_DIRS}
	${NMSmodel_INCLUDE_DIRS}
	${Curve_INCLUDE_DIRS}
)

add_definitions(/DBASE_DIR="${CMAKE_CURRENT_LIST_DIR}")

add_executable(Test_XmlInput xmlInputTest.cpp)
target_link_libraries(Test_XmlInput
	xmlInput
	ceinms_common
	NMSmodel
)

set_target_properties(Test_XmlInput PROPERTIES FOLDER "Tests")