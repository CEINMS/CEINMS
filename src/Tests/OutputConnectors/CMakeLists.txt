include_directories( 
    ${ceinms_common_INCLUDE_DIRS}
	${OPENSIMSIMBODY_INCLUDE_DIRS}
	${Plsr_INCLUDE_DIRS}
	${OutputConnectors_INCLUDE_DIRS}
    )

add_executable(Test_Websocket websocketTest.cpp)

TARGET_LINK_LIBRARIES(Test_Websocket       
        OutputConnectors ceinms_common)



