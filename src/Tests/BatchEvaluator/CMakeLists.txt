add_executable(Test_BatchEvaluator batchEvaluatorTest.cpp)
target_link_libraries(Test_BatchEvaluator
      xmlInput
      NMSmodel
      InputConnectors
      OutputConnectors
      OptimizationSystems
      rtb::Concurrency)

