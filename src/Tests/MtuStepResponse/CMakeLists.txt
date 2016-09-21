include_directories( 
	${Curve_INCLUDE_DIRS}
    ${ceinms_common_INCLUDE_DIRS}
    ${NMSmodel_INCLUDE_DIRS}
    ${FileIO_INCLUDE_DIRS}
    )

add_executable(Test_MtuStepResponse mtuStepResponseTest.cpp)

TARGET_LINK_LIBRARIES(Test_MtuStepResponse       
      NMSmodel)



