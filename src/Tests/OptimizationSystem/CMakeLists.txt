add_executable(Test_OptimizationSystems optimizationSystemsTest.cpp)
TARGET_LINK_LIBRARIES(Test_OptimizationSystems       
	  xmlInput
      NMSmodel
      InputConnectors
      OutputConnectors
      rtb::Concurrency
      OptimizationSystems
      ceinms_common)
