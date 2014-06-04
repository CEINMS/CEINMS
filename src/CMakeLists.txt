## Authors: Monica Reggiani, Claudio Pizzolato

include_directories(
  ${Producers_INCLUDE_DIRS}
  ${InputConnectors_INCLUDE_DIRS}
  ${NMSmodel_INCLUDE_DIRS}
  ${CMAKE_SOURCE_DIR}/lib/Curve
  ${CMAKE_SOURCE_DIR}/lib/Concurrency
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

set(CEINMS_COMMON_SOURCES ModelEvaluationBase.cpp ModelEvaluationHybrid.cpp ModelEvaluationOnline.cpp ModelEvaluationOffline.cpp)
set_source_files_properties(ModelEvaluationHybrid.cpp
                            ModelEvaluationOnline.cpp
                            ModelEvaluationOffline.cpp
                            PROPERTIES HEADER_FILE_ONLY TRUE)
set(CEINMS_HEADERS ModelEvaluationBase.h ModelEvaluationHybrid.h ModelEvaluationOnline.h ModelEvaluationOffline.h SimpleFileLogger.h)
set(CEINMS_SOURCES CEINMS.cpp ${CEINMS_COMMON_SOURCES})
ADD_EXECUTABLE( CEINMS ${CEINMS_SOURCES} ${CEINMS_HEADERS} )
TARGET_LINK_LIBRARIES( CEINMS
      FileIO
      xmlInput
      NMSmodel
      InputConnectors
      ${CMAKE_THREAD_LIBS_INIT}
      ${Boost_LIBRARIES}
)

#enable_language(Fortran)
#if(UNIX)
#find_package(LAPACK)
#set(lapack_unix ${LAPACK_LIBRARIES} ${BLAS_LIBRARIES})
#endif(UNIX)

if(OPENSIM_FOUND)
  include_directories(${OPENSIM_INCLUDE_DIRS} ${SIMTK_INCLUDE_DIRS})
  set(CEINMS-osim_SOURCES CEINMS-osim.cpp ProducersFromOpenSim.cpp ${CEINMS_COMMON_SOURCES})
  set(CEINMS-osim_HEADERS ProducersFromOpenSim.h ${CEINMS_HEADERS})
  add_executable(CEINMS-osim  ${CEINMS-osim_SOURCES} ${CEINMS-osim_HEADERS})

  target_link_libraries( CEINMS-osim
      FileIO
      xmlInput
      NMSmodel
      ProducersOSim
      ${CMAKE_THREAD_LIBS_INIT}
      ${Boost_LIBRARIES}
      ${OPENSIM_LIBRARIES}
  )

  if(UNIX)
  target_link_libraries( CEINMS-osim
	${lapack_unix} )
  endif(UNIX)

endif()

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
      ${Boost_LIBRARIES}
)

if(OPENSIM_FOUND)
  set(CEINMScalibrate-osim_SOURCES
    CEINMScalibration-osim.cpp  )
    
  add_executable( CEINMScalibrate-osim ${CEINMScalibrate-osim_SOURCES} )
  target_link_libraries( CEINMScalibrate-osim
      xmlInput
      Calibration
      NMSmodel
      FileIO
      ${Boost_LIBRARIES}
      ${XERCES_LIBRARY}
      ${OPENSIM_LIBRARIES}
  )

  if(UNIX)
  target_link_libraries( CEINMScalibrate-osim
	${lapack_unix} )
  endif(UNIX)
endif()
