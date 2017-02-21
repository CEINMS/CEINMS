## Authors: Monica Reggiani, Claudio Pizzolato

add_subdirectory(Tests)

include_directories(${Boost_INCLUDE_DIRS})

set(CEINMS_COMMON_SOURCES  ModelEvaluationHybrid.cpp ModelEvaluationOnline.cpp ModelEvaluationOffline.cpp ModelEvaluationBase.cpp SimulationManager.cpp)
set_source_files_properties(ModelEvaluationHybrid.cpp
                            ModelEvaluationOnline.cpp
                            ModelEvaluationOffline.cpp
                            ModelEvaluationBase.cpp
							SimulationManager.cpp
                            PROPERTIES HEADER_FILE_ONLY TRUE)
set(CEINMS_HEADERS ModelEvaluationBase.h 
                   ModelEvaluationHybrid.h 
                   ModelEvaluationOnline.h 
                   ModelEvaluationOffline.h 
                   Logger.h 
                   SimpleFileLogger.h 
                   StorageLogger.h 
                   SimulationManager.h)
                   
set(CEINMS_SOURCES CEINMS.cpp ${CEINMS_COMMON_SOURCES})
ADD_EXECUTABLE( CEINMS ${CEINMS_SOURCES} ${CEINMS_HEADERS} )
TARGET_LINK_LIBRARIES( CEINMS
      ceinms_common
      Curve
      FileIO
      xmlInput
      NMSmodel
      InputConnectors
      OutputConnectors
      Hybrid
      #${CMAKE_THREAD_LIBS_INIT}
      ${Boost_FILESYSTEM_LIBRARY}
)

set(CEINMScalibrate_HEADERS XmlWriter.h CalibrationManager.h)
set(CEINMScalibrate_COMMON_SOURCES XmlWriter.cpp CalibrationManager.cpp)
set(CEINMScalibrate_SOURCES CEINMScalibration.cpp ${CEINMScalibrate_COMMON_SOURCES})

set_source_files_properties(XmlWriter.cpp CalibrationManager.cpp PROPERTIES HEADER_FILE_ONLY TRUE)

add_executable( CEINMScalibrate ${CEINMScalibrate_SOURCES} ${CEINMScalibrate_HEADERS})
target_link_libraries( CEINMScalibrate
      ceinms_common
      xmlInput
      NMSmodel
      FileIO
      Curve
      InputConnectors
      OutputConnectors
      OptimizationSystems
      Optimizers
      TrialData
      Hybrid
)

install(TARGETS CEINMS CEINMScalibrate
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT Applications)

