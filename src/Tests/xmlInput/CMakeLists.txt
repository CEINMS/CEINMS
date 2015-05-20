include_directories(${xmlInput_INCLUDE_DIRS} ${ceinms_common_INCLUDE_DIRS}
					 )

add_executable(Test_XmlInput xmlInputTest.cpp)
target_link_libraries(Test_XmlInput xmlInput ceinms_common)
