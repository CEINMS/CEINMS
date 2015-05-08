# Author: Elena Ceseracciu

# Save path to include directory for "internal" usage
set(Hybrid_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}" CACHE INTERNAL "Include directory for Hybrid library")

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
# Install
install(FILES ${Hybrid_HEADERS} ${Hybrid_TEMPLATE_IMPLEMENTATIONS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/ceinms)
install(FILES ${Hybrid_Parameters_HEADERS} ${Hybrid_Parameters_TEMPLATE_IMPLEMENTATIONS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/ceinms/Parameters)
install(FILES ${Hybrid_StaticComp_HEADERS} ${Hybrid_StaticComp_TEMPLATE_IMPLEMENTATIONS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/ceinms/StaticComp)
install(FILES ${Hybrid_ObjFunc_HEADERS} ${Hybrid_ObjFunc_TEMPLATE_IMPLEMENTATIONS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/ceinms/ObjFunc)
