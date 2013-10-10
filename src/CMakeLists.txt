INCLUDE_DIRECTORIES(
  /usr/local/include/ 
  ${CMAKE_SOURCE_DIR}/lib/Producers
  ${CMAKE_SOURCE_DIR}/lib/Producers/FromFile
  ${CMAKE_SOURCE_DIR}/lib/NMSmodel
  ${CMAKE_SOURCE_DIR}/lib/NMSmodel/Tendon
  ${CMAKE_SOURCE_DIR}/lib/NMSmodel/Activation
  ${CMAKE_SOURCE_DIR}/lib/Curve
  ${CMAKE_SOURCE_DIR}/lib/xmlInput
  ${CMAKE_SOURCE_DIR}/lib/FileIO
  ${CMAKE_SOURCE_DIR}/lib/FileIO/EMGgenerator
  ${CMAKE_SOURCE_DIR}/lib/Calibration
  ${CMAKE_SOURCE_DIR}/lib/Calibration/SimAnnealParameters
  ${CMAKE_SOURCE_DIR}/lib/Calibration/ObjectiveFunctions
  ${CMAKE_SOURCE_DIR}/lib/Calibration/AmoebaParameters
  ${CMAKE_SOURCE_DIR}/lib/Calibration/TorqueComputationModes
  ${CMAKE_SOURCE_DIR}/lib/Hybrid
  ${CMAKE_SOURCE_DIR}/lib/common
)

LINK_DIRECTORIES(
  ${CMAKE_SOURCE_DIR}/lib/xmlInput
  ${CMAKE_SOURCE_DIR}/lib/NMSmodel
  ${CMAKE_SOURCE_DIR}/lib/FileIO
  ${CMAKE_SOURCE_DIR}/lib/Producers
  ${CMAKE_SOURCE_DIR}/lib/Calibration
  ${CMAKE_SOURCE_DIR}/lib/Hybrid
)



ADD_EXECUTABLE( CEINMS  CEINMS.cpp ModelEvaluationBase.cpp)
TARGET_LINK_LIBRARIES( CEINMS
      xmlInput
      NMSmodel
      FileIO
      Producers
      ${CMAKE_THREAD_LIBS_INIT}
      ${Boost_LIBRARIES}
      ${XERCES_LIBRARY}  
)


set(CEINMScalibrate_SOURCES
    CEINMScalibration.cpp
    ${CMAKE_SOURCE_DIR}/lib/Calibration/InputDataInterpreter.cpp
    
)
    
add_executable( CEINMScalibrate ${CEINMScalibrate_SOURCES})

TARGET_LINK_LIBRARIES( CEINMScalibrate
      xmlInputCalibration
      NMSmodel
      FileIO
      ${Boost_LIBRARIES}
      ${XERCES_LIBRARY}
)
