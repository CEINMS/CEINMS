## Authors: Monica Reggiani, Claudio Pizzolato

include_directories(
  ${CMAKE_SOURCE_DIR}/lib/Curve
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/Tendon
  ${CMAKE_CURRENT_SOURCE_DIR}/Activation
  ${CMAKE_SOURCE_DIR}/lib/OdeInt
  ${CMAKE_SOURCE_DIR}/lib/RootSolvers
  ${xmlInput_INCLUDE_DIRS}
  ${ceinms_common_INCLUDE_DIRS}
  ${FileIO_INCLUDE_DIRS}
)

include_directories(${Boost_INCLUDE_DIRS})

set(Tendon_SOURCES
  Tendon/ElasticTendon.cpp
  Tendon/ElasticTendon_BiSec.cpp
  Tendon/StiffTendon.cpp
) 
set(Tendon_HEADERS
  Tendon/ElasticTendon.h
  Tendon/ElasticTendon_BiSec.h
  Tendon/StiffTendon.h
)


set(Activation_SOURCES
  Activation/ExponentialActivation.cpp
  Activation/PiecewiseActivation.cpp
) 
set(Activation_HEADERS
  Activation/ExponentialActivation.h
  Activation/PiecewiseActivation.h
)

set(NMS_SOURCES NMSmodel.cpp MuscleParameters.cpp)
set(NMS_HEADERS NMSmodel.h MuscleParameters.h)


source_group("NMS_headers" FILES ${NMS_HEADERS})
source_group("NMS_sources" FILES ${NMS_SOURCES})
source_group("Tendon_headers" FILES ${Tendon_HEADERS})
source_group("Tendon_sources" FILES ${Tendon_SOURCES})
source_group("Activation_headers" FILES ${Activation_HEADERS})
source_group("Activation_sources" FILES ${Activation_SOURCES})

add_library(NMSmodel STATIC
  ${NMS_SOURCES}
  ${NMS_HEADERS}
  ${Tendon_SOURCES}
  ${Tendon_HEADERS}
  ${Activation_SOURCES}
  ${Activation_HEADERS}
)

set(NMSmodel_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/Activation ${CMAKE_CURRENT_SOURCE_DIR}/Tendon CACHE STRING "Include directory for NMSmodel library")
mark_as_advanced(NMSmodel_INCLUDE_DIRS)

install(FILES ${Activation_HEADERS} ${NMS_HEADERS} ${Tendon_HEADERS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}) #should add namespace
install(TARGETS NMSmodel
        EXPORT CEINMSTargets
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})

