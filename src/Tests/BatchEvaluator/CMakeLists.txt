include_directories(OptimizationSystems 
	${OptimizationSystems_INCLUDE_DIRS}   
	${InputConnectors_INCLUDE_DIRS}
	${OutputConnectors_INCLUDE_DIRS}
	${xmlInput_INCLUDE_DIRS})

add_executable(Test_BatchEvaluator batchEvaluatorTest.cpp)
TARGET_LINK_LIBRARIES(Test_BatchEvaluator       
	  xmlInput
      NMSmodel
      InputConnectors
      OutputConnectors)

