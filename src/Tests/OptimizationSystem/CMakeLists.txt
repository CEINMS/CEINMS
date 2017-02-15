include_directories( 
	${NMSmodel_INCLUDE_DIRS}
	${OptimizationSystems_INCLUDE_DIRS} 
	${Optimizers_INCLUDE_DIRS}
	${InputConnectors_INCLUDE_DIRS}
	${OutputConnectors_INCLUDE_DIRS}
	${xmlInput_INCLUDE_DIRS}
	${ceinms_common_INCLUDE_DIRS}
	${TrialData_INCLUDE_DIRS}
	${FileIO_INCLUDE_DIRS}
	${Curve_INCLUDE_DIRS})

add_executable(Test_OptimizationSystems optimizationSystemsTest.cpp)
TARGET_LINK_LIBRARIES(Test_OptimizationSystems       
	  xmlInput
      NMSmodel
      InputConnectors
      OutputConnectors
      rtb::Concurrency
      OptimizationSystems
      ceinms_common)

