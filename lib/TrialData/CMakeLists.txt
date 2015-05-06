# Authors: Elena Ceseracciu, Monica Reggiani, Claudio Pizzolato

project(TrialData)

# List source files
set(TrialData_HEADERS DataTable.h
                      TrialData.h
                      )

set(TrialData_TEMPLATE_IMPLEMENTATION
                            DataTable.cpp)

set_source_files_properties( ${TrialData_TEMPLATE_IMPLEMENTATION} PROPERTIES HEADER_FILE_ONLY TRUE)

source_group("Header Files" FILES ${TrialData_HEADERS})
source_group("Source Files" FILES ${TrialData_TEMPLATE_IMPLEMENTATION})

add_library(TrialData ${TrialData_HEADERS} ${TrialData_TEMPLATE_IMPLEMENTATION})



# Save path to include directory for "internal" usage
set(TrialData_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR} 
#${CMAKE_CURRENT_SOURCE_DIR}/ObjectiveFunctions ${CMAKE_CURRENT_SOURCE_DIR}/SimAnnealParameters  ${CMAKE_CURRENT_SOURCE_DIR}/TorqueComputationModes 
CACHE INTERNAL "Include directories for TrialData library")

#Install
#install(FILES ${OptimizationSystems_HEADERS} ${OptimizationSystems_TEMPLATE_IMPLEMENTATION} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}) #should add namespace
#install(TARGETS OptimizationSystems
#        EXPORT CEINMSTargets
#        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
#        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
#        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
