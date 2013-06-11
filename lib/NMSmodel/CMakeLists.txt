INCLUDE_DIRECTORIES(
  ${CMAKE_SOURCE_DIR}/lib/xmlInput 
  ${CMAKE_SOURCE_DIR}/lib/Curve
  ${CMAKE_SOURCE_DIR}/lib/NMSmodel
  ${CMAKE_SOURCE_DIR}/lib/NMSmodel/Tendon
  ${CMAKE_SOURCE_DIR}/lib/NMSmodel/Activation
  ${CMAKE_SOURCE_DIR}/lib/OdeInt
)

SET(Tendon_SOURCES
  Tendon/ElasticTendon.cpp
  Tendon/ElasticTendon_BiSec.cpp
  Tendon/StiffTendon.cpp
) 


SET(Activation_SOURCES
  Activation/ExponentialActivation.cpp
  Activation/PiecewiseActivation.cpp
) 

ADD_LIBRARY(NMSmodel STATIC
  NMSmodel.cpp MuscleParameters.cpp 
  ${Tendon_SOURCES}
  ${Activation_SOURCES}
)

