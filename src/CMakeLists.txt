## Authors: Monica Reggiani, Claudio Pizzolato

include_directories(
  ${Producers_INCLUDE_DIRS}
  ${NMSmodel_INCLUDE_DIRS}
  ${CMAKE_SOURCE_DIR}/lib/Curve
  ${xmlInput_INCLUDE_DIRS}
  ${FileIO_INCLUDE_DIRS}
  ${Calibration_INCLUDE_DIRS}
  ${CMAKE_SOURCE_DIR}/lib/Hybrid
  ${ceinms_common_INCLUDE_DIRS}
)
include_directories(${Boost_INCLUDE_DIRS})

include_directories(${OPENSIM_INCLUDE_DIRS} ${SIMTK_INCLUDE_DIRS})

#LINK_DIRECTORIES(
#  ${CMAKE_SOURCE_DIR}/lib/xmlInput
#  ${CMAKE_SOURCE_DIR}/lib/NMSmodel
#  ${CMAKE_SOURCE_DIR}/lib/FileIO
#  ${CMAKE_SOURCE_DIR}/lib/Producers
#  ${CMAKE_SOURCE_DIR}/lib/Calibration
#  ${CMAKE_SOURCE_DIR}/lib/Hybrid
#)



ADD_EXECUTABLE( CEINMS  CEINMS.cpp ModelEvaluationBase.cpp)
TARGET_LINK_LIBRARIES( CEINMS
      FileIO
      xmlInput
      NMSmodel
      Producers
      ${CMAKE_THREAD_LIBS_INIT}
      ${Boost_LIBRARIES}
)

#enable_language(Fortran)
#if(UNIX)
#find_package(LAPACK)
#set(lapack_unix ${LAPACK_LIBRARIES} ${BLAS_LIBRARIES})
#endif(UNIX)

if(OPENSIM_FOUND)
  add_executable( CEINMS-osim  CEINMS-osim.cpp ModelEvaluationBase.cpp ProducersFromOpenSim.cpp)

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

set(CEINMScalibrate_SOURCES
    CEINMScalibration.cpp
)

add_executable( CEINMScalibrate ${CEINMScalibrate_SOURCES})
target_link_libraries( CEINMScalibrate
      xmlInputCalibration
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
      xmlInputCalibration
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
