## Authors: Monica Reggiani, Claudio Pizzolato

# List source files
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

set(NMS_SOURCES NMSmodel.cpp MuscleParameters.cpp MTUutils.cpp)
set(NMS_HEADERS NMSmodel.h MuscleParameters.h MTU.h DoF.h SetupDataStructure.h NMSmodelPolicyTemplates.h MTUutils.h)
set(NMS_TEMPLATE_IMPLEMENTATION MTU.cpp DoF.cpp SetupDataStructure.cpp)
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
target_include_directories(NMSmodel PRIVATE ${Curve_INCLUDE_DIRS}
                                            Tendon
                                            Activation
                                            ${OdeInt_INCLUDE_DIRS}
                                            ${RootSolvers_INCLUDE_DIRS}
                                            ${xmlInput_INCLUDE_DIRS}
                                            ${ceinms_common_INCLUDE_DIRS}
                                            ${FileIO_INCLUDE_DIRS}
#                                    PUBLIC  ${Boost_INCLUDE_DIRS}
                                    )
target_link_libraries(NMSmodel xmlInput)
# Save path to include directory for "internal" usage
set(NMSmodel_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/Activation" "${CMAKE_CURRENT_SOURCE_DIR}/Tendon" CACHE STRING "Include directory for NMSmodel library")
mark_as_advanced(NMSmodel_INCLUDE_DIRS)

# Install
install(FILES ${Activation_HEADERS} ${NMS_HEADERS} ${Tendon_HEADERS} ${NMS_TEMPLATE_IMPLEMENTATION} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}) #should add namespace
install(TARGETS NMSmodel
        EXPORT CEINMSTargets
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})

