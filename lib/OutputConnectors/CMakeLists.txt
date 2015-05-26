## Authors: Monica Reggiani, Claudio Pizzolato, Elena Ceseracciu

# List source files
set(OutputConnectors_SOURCES
    OutputConnectors.cpp
    LoggerOnQueues.cpp
    QueuesToTrialData.cpp
    QueuesToXForces.cpp
    QueuesToJointReactionForces.cpp
    QueuesToIntersegmentalForces.cpp
    QueuesToIntersegmentalAndJointReactionForces.cpp
	QueuesToKneeBalanceContactForces.cpp
)
set(OutputConnectors_HEADERS 
    ceinms/OutputConnectors.h
    ceinms/LoggerOnQueues.h
    ceinms/QueuesToTrialData.h
    ceinms/QueuesToXForces.h
    ceinms/QueuesToContactForces.h
    ceinms/QueuesToJointReactionForces.h
    ceinms/QueuesToIntersegmentalForces.h
    ceinms/QueuesToIntersegmentalAndJointReactionForces.h
	ceinms/QueuesToKneeBalanceContactForces.h
)

set(QueuesToStorageFiles_SOURCES
    QueuesToStorageFiles/QueuesToStorageFiles.cpp
    
  
)

set(QueuesToStorageFiles_HEADERS
    ceinms/QueuesToStorageFiles.h
    ceinms/QueueToStorageFile.h
)

set(QueuesToStorageFiles_TEMPLATE_IMPLEMENTATION
	QueuesToStorageFiles/QueueToStorageFile.cpp
	)

set(OutputConnectors_TEMPLATE_IMPLEMENTATION
	QueuesToContactForces.cpp
	)
  
set_source_files_properties( ${OutputConnectors_TEMPLATE_IMPLEMENTATION} PROPERTIES HEADER_FILE_ONLY TRUE)
set_source_files_properties( ${QueuesToStorageFiles_TEMPLATE_IMPLEMENTATION} PROPERTIES HEADER_FILE_ONLY TRUE)


source_group("Header Files" FILES ${OutputConnectors_HEADERS} ${QueuesToStorageFiles_HEADERS})
source_group("Source Files" FILES ${OutputConnectors_TEMPLATE_IMPLEMENTATION} ${QueuesToStorageFiles_SOURCES} ${QueuesToStorageFiles_TEMPLATE_IMPLEMENTATION})


# Define target
add_library(OutputConnectors STATIC
  ${OutputConnectors_SOURCES}
  ${OutputConnectors_HEADERS}
  ${OutputConnectors_TEMPLATE_IMPLEMENTATION}
  ${QueuesToStorageFiles_TEMPLATE_IMPLEMENTATION}
  ${QueuesToStorageFiles_SOURCES}
  ${QueuesToStorageFiles_HEADERS}
)
target_link_libraries(OutputConnectors JointContactModels ceinms_common ${Boost_FILESYSTEM_LIBRARY} optimized ${OPENSIM_LIBRARIES})
target_link_libraries(OutputConnectors JointContactModels ceinms_common ${Boost_FILESYSTEM_LIBRARY} debug ${OPENSIM_LIBRARIES_DEBUG})

target_include_directories(OutputConnectors
                                PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
                                        ${ceinms_common_INCLUDE_DIRS}
                                        ${InputConnectors_INCLUDE_DIRS}
                                        ${Concurrency_INCLUDE_DIRS}
                                        ${TrialData_INCLUDE_DIRS}
										${JointContactModels_INCLUDE_DIRS}
                                PUBLIC  ${Boost_INCLUDE_DIRS}
								        ${OPENSIM_INCLUDE_DIRS}
                                        ${SIMTK_INCLUDE_DIRS})

# Save path to include directory for "internal" usage
set(OutputConnectors_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}" CACHE INTERNAL "Include directories for OutputConnectors library")

# Install
install(FILES ${OutputConnectors_HEADERS} ${QueuesToStorageFiles_HEADERS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/ceinms)
install(TARGETS OutputConnectors
        EXPORT CEINMSTargets
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
