## Authors: Monica Reggiani, Claudio Pizzolato

include_directories(
  #${Producers_INCLUDE_DIRS}
  ${InputConnectors_INCLUDE_DIRS}
  ${OutputConnectors_INCLUDE_DIRS}
  ${NMSmodel_INCLUDE_DIRS}
  ${Curve_INCLUDE_DIRS}
  ${Concurrency_INCLUDE_DIRS}
  ${xmlInput_INCLUDE_DIRS}
  ${FileIO_INCLUDE_DIRS}
  ${Calibration_INCLUDE_DIRS}
  ${CMAKE_SOURCE_DIR}/lib/Hybrid
  ${ceinms_common_INCLUDE_DIRS}
)
include_directories(${Boost_INCLUDE_DIRS})

#LINK_DIRECTORIES(
#  ${CMAKE_SOURCE_DIR}/lib/xmlInput
#  ${CMAKE_SOURCE_DIR}/lib/NMSmodel
#  ${CMAKE_SOURCE_DIR}/lib/FileIO
#  ${CMAKE_SOURCE_DIR}/lib/Producers
#  ${CMAKE_SOURCE_DIR}/lib/Calibration
#  ${CMAKE_SOURCE_DIR}/lib/Hybrid
#)

set(CEINMS_COMMON_SOURCES  ModelEvaluationHybrid.cpp ModelEvaluationOnline.cpp ModelEvaluationOffline.cpp)
set_source_files_properties(ModelEvaluationHybrid.cpp
                            ModelEvaluationOnline.cpp
                            ModelEvaluationOffline.cpp
                            PROPERTIES HEADER_FILE_ONLY TRUE)
set(CEINMS_HEADERS ModelEvaluationBase.h ModelEvaluationHybrid.h ModelEvaluationOnline.h ModelEvaluationOffline.h Logger.h SimpleFileLogger.h StorageLogger.h)
set(CEINMS_SOURCES CEINMS.cpp ${CEINMS_COMMON_SOURCES})
ADD_EXECUTABLE( CEINMS ${CEINMS_SOURCES} ${CEINMS_HEADERS} )
TARGET_LINK_LIBRARIES( CEINMS
      FileIO
      xmlInput
      NMSmodel
      InputConnectors
      OutputConnectors
      #${CMAKE_THREAD_LIBS_INIT}
      ${Boost_PROGRAM_OPTIONS_LIBRARY}
      ${Boost_FILESYSTEM_LIBRARY}
)

ADD_EXECUTABLE( TestInputConnectors EXCLUDE_FROM_ALL TestInputConnectors.cpp  ${CEINMS_COMMON_SOURCES} ${CEINMS_HEADERS}  )
TARGET_LINK_LIBRARIES( TestInputConnectors
      FileIO
      xmlInput
      NMSmodel
      InputConnectors
      #${CMAKE_THREAD_LIBS_INIT}
      ${Boost_PROGRAM_OPTIONS_LIBRARY}
      ${Boost_FILESYSTEM_LIBRARY}
)


set(CEINMScalibrate_HEADERS XmlWriter.h)
set(CEINMScalibrate_COMMON_SOURCES XmlWriter.cpp)
set(CEINMScalibrate_SOURCES CEINMScalibration.cpp ${CEINMScalibrate_COMMON_SOURCES})

set_source_files_properties(XmlWriter.cpp PROPERTIES HEADER_FILE_ONLY TRUE)

add_executable( CEINMScalibrate ${CEINMScalibrate_SOURCES} ${CEINMScalibrate_HEADERS})
target_link_libraries( CEINMScalibrate
      xmlInput
      Calibration
      NMSmodel
      FileIO
      InputConnectors
      OutputConnectors
      ${Boost_PROGRAM_OPTIONS_LIBRARY}
)


install(TARGETS CEINMS CEINMScalibrate
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

