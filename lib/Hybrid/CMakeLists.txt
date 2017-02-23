# Author: Elena Ceseracciu

set(Hybrid_HEADERS  ceinms/SimulatedAnnealingBase.h
                    ceinms/StaticComputation.h
                    ceinms/HybridWeightings.h)

set(Hybrid_Parameters_HEADERS ceinms/Parameters/RecursiveEMGs.h)
set(Hybrid_StaticComp_HEADERS ceinms/StaticComputationMode/Default.h)
set(Hybrid_ObjFunc_HEADERS   ceinms/ObjectiveFunction/MinTorqueTrackEMGs.h)

set(Hybrid_TEMPLATE_IMPLEMENTATIONS ceinms/SimulatedAnnealingBase.cpp
                                    ceinms/StaticComputation.cpp)
set(Hybrid_Parameters_TEMPLATE_IMPLEMENTATIONS ceinms/Parameters/RecursiveEMGs.cpp)
set(Hybrid_StaticComp_TEMPLATE_IMPLEMENTATIONS ceinms/StaticComputationMode/Default.cpp)
set(Hybrid_ObjFunc_TEMPLATE_IMPLEMENTATIONS    ceinms/ObjectiveFunction/MinTorqueTrackEMGs.cpp)

add_library(Hybrid INTERFACE)
add_custom_target(Hybrid_IDE SOURCES ${Hybrid_HEADERS} ${Hybrid_TEMPLATE_IMPLEMENTATIONS}
                            ${Hybrid_Parameters_HEADERS} ${Hybrid_Parameters_TEMPLATE_IMPLEMENTATIONS}
                            ${Hybrid_StaticComp_HEADERS} ${Hybrid_StaticComp_TEMPLATE_IMPLEMENTATIONS}
                            ${Hybrid_ObjFunc_HEADERS} ${Hybrid_ObjFunc_TEMPLATE_IMPLEMENTATIONS})

target_include_directories(Hybrid INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
                                            $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
# Install
install(FILES ${Hybrid_HEADERS} ${Hybrid_TEMPLATE_IMPLEMENTATIONS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/ceinms COMPONENT Development)
install(FILES ${Hybrid_Parameters_HEADERS} ${Hybrid_Parameters_TEMPLATE_IMPLEMENTATIONS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/ceinms/Parameters COMPONENT Development)
install(FILES ${Hybrid_StaticComp_HEADERS} ${Hybrid_StaticComp_TEMPLATE_IMPLEMENTATIONS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/ceinms/StaticComp COMPONENT Development)
install(FILES ${Hybrid_ObjFunc_HEADERS} ${Hybrid_ObjFunc_TEMPLATE_IMPLEMENTATIONS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/ceinms/ObjFunc COMPONENT Development)

install(TARGETS Hybrid
        EXPORT CEINMSTargets)
