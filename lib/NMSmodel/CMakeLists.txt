## Authors: Monica Reggiani, Claudio Pizzolato

# List source files
set(Tendon_SOURCES
  Tendon/ElasticTendon.cpp
  Tendon/ElasticTendon_BiSec.cpp
  Tendon/StiffTendon.cpp
)
set(Tendon_HEADERS
  ceinms/Tendon/ElasticTendon.h
  ceinms/Tendon/ElasticTendon_BiSec.h
  ceinms/Tendon/StiffTendon.h
)

set(Activation_SOURCES
  Activation/ExponentialActivation.cpp
  Activation/PiecewiseActivation.cpp
) 
set(Activation_HEADERS
  ceinms/Activation/ExponentialActivation.h
  ceinms/Activation/PiecewiseActivation.h
)

set(NMS_SOURCES NMSmodel.cpp MuscleParameters.cpp MTUutils.cpp)
set(NMS_HEADERS ceinms/NMSmodel.h
                ceinms/MuscleParameters.h
                ceinms/MTU.h
                ceinms/DoF.h
                ceinms/SetupDataStructure.h
                ceinms/NMSmodelPolicyTemplates.h
                ceinms/MTUutils.h)

set(NMS_TEMPLATE_IMPLEMENTATION ceinms/MTU.cpp
                                ceinms/DoF.cpp
                                ceinms/SetupDataStructure.cpp)

set_source_files_properties( ${NMS_TEMPLATE_IMPLEMENTATION} PROPERTIES HEADER_FILE_ONLY TRUE)

source_group("NMS_headers" FILES ${NMS_HEADERS})
source_group("NMS_sources" FILES ${NMS_SOURCES})
source_group("NMS_template_impl" FILES ${NMS_TEMPLATE_IMPLEMENTATION})
source_group("Tendon_headers" FILES ${Tendon_HEADERS})
source_group("Tendon_sources" FILES ${Tendon_SOURCES})
source_group("Activation_headers" FILES ${Activation_HEADERS})
source_group("Activation_sources" FILES ${Activation_SOURCES})

# Define target
add_library(NMSmodel STATIC
  ${NMS_SOURCES}
  ${NMS_HEADERS}
  ${NMS_TEMPLATE_IMPLEMENTATION}
  ${Tendon_SOURCES}
  ${Tendon_HEADERS}
  ${Activation_SOURCES}
  ${Activation_HEADERS}
)
target_include_directories(NMSmodel PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
                                        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
target_link_libraries(NMSmodel PUBLIC xmlInput Curve ceinms_common FileIO PRIVATE OdeInt RootSolvers)

# Install
install(FILES ${NMS_HEADERS} ${NMS_TEMPLATE_IMPLEMENTATION} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/ceinms COMPONENT Development)
install(FILES ${Activation_HEADERS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/ceinms/Activation COMPONENT Development)
install(FILES ${Tendon_HEADERS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/ceinms/Tendon COMPONENT Development)
install(TARGETS NMSmodel
        EXPORT CEINMSTargets
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT Applications
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT Development
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT Development)

