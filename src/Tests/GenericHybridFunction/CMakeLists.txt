include_directories( 
	${Curve_INCLUDE_DIRS}
    ${ceinms_common_INCLUDE_DIRS}
    ${NMSmodel_INCLUDE_DIRS}
    ${FileIO_INCLUDE_DIRS}
    )

add_executable(Test_GenericHybridFunction genericHybridFunctionTest.cpp)

TARGET_LINK_LIBRARIES(Test_GenericHybridFunction       
      NMSmodel)



