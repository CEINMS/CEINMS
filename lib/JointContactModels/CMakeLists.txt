# Authors: Elena Ceseracciu, Monica Reggiani, Claudio Pizzolato

project(JointContactModels)

# List source files
set(JointContactModels_HEADERS 
    ceinms/JointContactFromOpensim.h                            
    ceinms/ContactModelKneeBalance.h
    )

set(JointContactModels_TEMPLATE_IMPLEMENTATION

							)
                            
set(JointContactModels_SOURCES 
    JointContactFromOpensim.cpp
    ContactModelKneeBalance.cpp
)

set_source_files_properties( ${JointContactModels_TEMPLATE_IMPLEMENTATION} PROPERTIES HEADER_FILE_ONLY TRUE)

source_group("Header Files" FILES ${JointContactModels_HEADERS})
source_group("Source Files" FILES ${JointContactModels_TEMPLATE_IMPLEMENTATION} ${JointContactModels_SOURCES})

add_library(JointContactModels STATIC ${JointContactModels_HEADERS} ${JointContactModels_TEMPLATE_IMPLEMENTATION} ${JointContactModels_SOURCES})
target_include_directories( JointContactModels
                            PRIVATE 
                                   ${OPENSIM_INCLUDE_DIRS}       
                                   ${SIMTK_INCLUDE_DIRS}
                            #       ${NMSmodel_INCLUDE_DIRS}
                             #      ${Curve_INCLUDE_DIRS}
                              #     ${FileIO_INCLUDE_DIRS}
                                   ${ceinms_common_INCLUDE_DIRS}
								#	${Optimizers_INCLUDE_DIRS}
								#	${TrialData_INCLUDE_DIRS}
                                    )

target_link_libraries(JointContactModels optimized ${OPENSIM_LIBRARIES})
target_link_libraries(JointContactModels debug ${OPENSIM_LIBRARIES_DEBUG})

# Save path to include directory for "internal" usage
set(JointContactModels_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}" CACHE INTERNAL "Include directory for JointContactModels library")
#mark_as_advanced(JointContactModels_INCLUDE_DIRS)

# Install
install(FILES ${JointContactModels_HEADERS} ${JointContactModels_TEMPLATE_IMPLEMENTATION} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/ceinms)
install(TARGETS JointContactModels
        EXPORT CEINMSTargets
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})