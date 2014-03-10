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

set(Tendon_SOURCES
  Tendon/ElasticTendon.cpp
  Tendon/ElasticTendon_BiSec.cpp
  Tendon/StiffTendon.cpp
) 


set(Activation_SOURCES
  Activation/ExponentialActivation.cpp
  Activation/PiecewiseActivation.cpp
) 

add_library(NMSmodel STATIC
  NMSmodel.cpp MuscleParameters.cpp 
  ${Tendon_SOURCES}
  ${Activation_SOURCES}
)

set(NMSmodel_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/Activation ${CMAKE_CURRENT_SOURCE_DIR}/Tendon CACHE STRING "Include directory for NMSmodel library")

