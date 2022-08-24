## Authors: Monica Reggiani, Claudio Pizzolato

add_subdirectory(Tests)
#add_subdirectory(real-time)

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

# CEINMS Execution
set(CEINMS_COMMON_SOURCES  SimulationManager.cpp)
set_source_files_properties(SimulationManager.cpp PROPERTIES HEADER_FILE_ONLY TRUE)
set(CEINMS_SOURCES CEINMS.cpp ${CEINMS_COMMON_SOURCES})
set(CEINMS_HEADERS 
      Logger.h 
      SimpleFileLogger.h 
      StorageLogger.h 
      SimulationManager.h
)                 

add_executable(CEINMS ${CEINMS_SOURCES} ${CEINMS_HEADERS})
target_link_libraries(CEINMS
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

# CEINMS Calibrate
set(CEINMScalibrate_COMMON_SOURCES CalibrationManager.cpp)
set_source_files_properties(CalibrationManager.cpp PROPERTIES HEADER_FILE_ONLY TRUE)
set(CEINMScalibrate_SOURCES CEINMScalibration.cpp ${CEINMScalibrate_COMMON_SOURCES})
set(CEINMScalibrate_HEADERS CalibrationManager.h)

add_executable(CEINMScalibrate ${CEINMScalibrate_SOURCES} ${CEINMScalibrate_HEADERS})
target_link_libraries(CEINMScalibrate
      xmlInput
      NMSmodel
      FileIO
      InputConnectors
      OutputConnectors
      OptimizationSystems
	TrialData
	rtb::Concurrency
)

# CEINMS Optimise
add_executable(CEINMSoptimise CEINMSoptimise.cpp SimulationManager.cpp SimulationManager.h)
target_link_libraries(CEINMSoptimise
      FileIO
      xmlInput
      NMSmodel
      InputConnectors
      OutputConnectors
	JointContactModels
      OptimizationSystems
      TrialData
	rtb::Concurrency
)

# Install
install(TARGETS CEINMS CEINMScalibrate CEINMSoptimise
      RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT Applications
      LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT Development
      ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT Development
)
