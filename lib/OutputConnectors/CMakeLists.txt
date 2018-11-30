## Authors: Monica Reggiani, Claudio Pizzolato, Elena Ceseracciu

# List source files
set(OutputConnectors_SOURCES
    LoggerOnQueues.cpp
    QueuesToTrialData.cpp
    QueuesToXForces.cpp
    QueuesToJointReactionForces.cpp
    QueuesToIntersegmentalForces.cpp
    QueuesToIntersegmentalAndJointReactionForces.cpp
	QueuesToKneeBalanceContactForces.cpp
    QueuesToInverseDynamics.cpp
    QueuesToStorageFiles.cpp
	websocket/QueuesToWebsocket.cpp
)
set(OutputConnectors_HEADERS 
    ceinms/LoggerOnQueues.h
    ceinms/QueuesToTrialData.h
    ceinms/QueuesToXForces.h
    ceinms/QueuesToJointReactionForces.h
    ceinms/QueuesToIntersegmentalForces.h
    ceinms/QueuesToIntersegmentalAndJointReactionForces.h
	ceinms/QueuesToKneeBalanceContactForces.h
    ceinms/QueuesToInverseDynamics.h
    ceinms/QueuesToStorageFiles.h
    ceinms/QueueToStorageFile.h
	ceinms/websocket/QueuesToWebsocket.h
	ceinms/websocket/communication.h
)


set(OutputConnectors_TEMPLATE_IMPLEMENTATION
    ceinms/QueueToStorageFile.cpp

	)
  
find_package(protobuf CONFIG REQUIRED)
set(Protobuf_FILES websocket/protobuf/yabbi.proto)
set_source_files_properties(${Proto_SRC} ${Proto_HEADERS} PROPERTIES GENERATED TRUE)
protobuf_generate_cpp(Proto_SRC Proto_HEADERS ${Protobuf_FILES})
if(${CMAKE_VERBOSE_MAKEFILE})
	message(STATUS "Proto_SRC=${Proto_SRC}")
	message(STATUS "Proto_HEADERS=${Proto_HEADERS}")
endif()
set(GeneratedProtobuf_INCLUDE_DIRS ${CMAKE_CURRENT_BINARY_DIR}
    CACHE INTERNAL "Path to generated protobuf files.")

set_source_files_properties( ${OutputConnectors_TEMPLATE_IMPLEMENTATION} PROPERTIES HEADER_FILE_ONLY TRUE)
set_source_files_properties( ${QueuesToStorageFiles_TEMPLATE_IMPLEMENTATION} PROPERTIES HEADER_FILE_ONLY TRUE)
set_source_files_properties( ${Protobuf_FILES} PROPERTIES EXTERNAL_OBJECT TRUE)


source_group("Header Files" FILES ${OutputConnectors_HEADERS} ${QueuesToStorageFiles_HEADERS} )
source_group("Source Files" FILES ${OutputConnectors_TEMPLATE_IMPLEMENTATION} ${QueuesToStorageFiles_SOURCES} 
	${QueuesToStorageFiles_TEMPLATE_IMPLEMENTATION} )
source_group("Generated headers" FILES ${Proto_HEADERS})
source_group("Generated sources" FILES ${Proto_SRC})
source_group("Protobuffers" FILES ${Protobuf_FILES})


# Define target
add_library(OutputConnectors STATIC
  ${OutputConnectors_SOURCES}
  ${OutputConnectors_HEADERS}
  ${OutputConnectors_TEMPLATE_IMPLEMENTATION}
  ${QueuesToStorageFiles_TEMPLATE_IMPLEMENTATION}
  ${QueuesToStorageFiles_SOURCES}
  ${QueuesToStorageFiles_HEADERS}
  ${Proto_HEADERS}
  ${Proto_SRC}
)
target_link_libraries(OutputConnectors JointContactModels ceinms_common ${Boost_LIBRARIES}
	protobuf::libprotoc protobuf::libprotobuf protobuf::libprotobuf-lite optimized ${OPENSIM_LIBRARIES})
#target_link_libraries(OutputConnectors JointContactModels ceinms_common ${Boost_FILESYSTEM_LIBRARY} debug ${OPENSIM_LIBRARIES_DEBUG})

target_include_directories(OutputConnectors
                                PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
                                        ${ceinms_common_INCLUDE_DIRS}
                                        ${InputConnectors_INCLUDE_DIRS}
                                        ${Concurrency_INCLUDE_DIRS}
                                        ${TrialData_INCLUDE_DIRS}
										${JointContactModels_INCLUDE_DIRS}
										${FileIO_INCLUDE_DIRS}
										${GeneratedProtobuf_INCLUDE_DIRS}
                                PUBLIC  ${Boost_INCLUDE_DIRS}
								        ${OPENSIM_INCLUDE_DIRS}
                                        ${SIMTK_INCLUDE_DIRS})

# Save path to include directory for "internal" usage
set(OutputConnectors_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}" CACHE INTERNAL "Include directories for OutputConnectors library")

# Install

install(FILES ${OutputConnectors_HEADERS} ${OutputConnectors_TEMPLATE_IMPLEMENTATION} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/ceinms COMPONENT Development)
install(TARGETS OutputConnectors
        EXPORT CEINMSTargets
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT Applications
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT Development
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT Development)
