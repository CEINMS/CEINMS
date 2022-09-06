include_directories(
      ${InputConnectors_INCLUDE_DIRS}
      ${OutputConnectors_INCLUDE_DIRS}
      ${NMSmodel_INCLUDE_DIRS}
      ${Curve_INCLUDE_DIRS}
      ${Concurrency_INCLUDE_DIRS}
      ${xmlInput_INCLUDE_DIRS}
      ${FileIO_INCLUDE_DIRS}
      ${CMAKE_SOURCE_DIR}/lib/Hybrid
      ${ceinms_common_INCLUDE_DIRS}
      ${Optimizers_INCLUDE_DIRS}
      ${OptimizationSystems_INCLUDE_DIRS}
      ${TrialData_INCLUDE_DIRS}
      ${JointContactModels_INCLUDE_DIRS}
      ${ModelEvaluators_INCLUDE_DIRS}
)
set(CEINMS_COMMON_SOURCES  SimulationManager.cpp)
set_source_files_properties(SimulationManager.cpp PROPERTIES HEADER_FILE_ONLY TRUE)
set(CEINMS_SOURCES executionTest.cpp ${CEINMS_COMMON_SOURCES})
set(CEINMS_HEADERS 
      Logger.h 
      SimpleFileLogger.h 
      StorageLogger.h 
      SimulationManager.h
)       
add_definitions(/DBASE_DIR="${CMAKE_CURRENT_LIST_DIR}")
add_executable(Test_CEINMSExecution ${CEINMS_SOURCES} ${CEINMS_HEADERS})
target_link_libraries(Test_CEINMSExecution
      FileIO
      xmlInput
      NMSmodel
      InputConnectors
      OutputConnectors
      JointContactModels
      TrialData
      OptimizationSystems
	  ceinms_common
)
set_target_properties(Test_CEINMSExecution PROPERTIES FOLDER "Tests")
add_test(NAME TestCEINMSExecution COMMAND Test_CEINMSExecution)
