#include_directories(${xmlInput_INCLUDE_DIRS} ${ceinms_common_INCLUDE_DIRS}
#					 )
#add_definitions(/DBASE_DIR="${CMAKE_CURRENT_LIST_DIR}")
add_executable(Test_Barrier barrierTest.cpp)
#target_link_libraries(Test_XmlInput xmlInput ceinms_common)
