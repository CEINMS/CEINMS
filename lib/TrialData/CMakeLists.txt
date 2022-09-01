# Authors: Elena Ceseracciu, Monica Reggiani, Claudio Pizzolato

project(TrialData)

# List source files
set(TrialData_HEADERS
        ceinms/DataTable.h
        ceinms/TrialData.h
)

set(TrialData_TEMPLATE_IMPLEMENTATION
        ceinms/DataTable.cpp
)

set(TrialData_SOURCE
        TrialData.cpp
)

set_source_files_properties( ${TrialData_TEMPLATE_IMPLEMENTATION} PROPERTIES HEADER_FILE_ONLY TRUE)

source_group("Header Files" FILES ${TrialData_HEADERS})
source_group("Source Files" FILES ${TrialData_TEMPLATE_IMPLEMENTATION} ${TrialData_SOURCE})

add_library(TrialData ${TrialData_HEADERS} ${TrialData_TEMPLATE_IMPLEMENTATION} ${TrialData_SOURCE})
target_link_libraries(TrialData ${OpenSim_LIBRARIES})
target_include_directories(TrialData
                        PRIVATE
                                # ${NMSmodel_INCLUDE_DIRS}
                                # ${Curve_INCLUDE_DIRS}
                                # ${FileIO_INCLUDE_DIRS}
                                ${ceinms_common_INCLUDE_DIRS}
				# ${Optimizers_INCLUDE_DIRS}
				# ${TrialData_INCLUDE_DIRS}
)

# Save path to include directory for "internal" usage
set(TrialData_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}"
        CACHE INTERNAL "Include directories for TrialData library")

#Install
install(FILES ${TrialData_HEADERS} ${TrialData_TEMPLATE_IMPLEMENTATION} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/ceinms COMPONENT Development) 
#install(TARGETS TrialData
#        EXPORT CEINMSTargets
#        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT Applications
#        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT Development
#        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT Development)

set_target_properties(TrialData PROPERTIES FOLDER "Libraries")