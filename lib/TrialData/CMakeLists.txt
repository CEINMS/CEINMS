# Authors: Elena Ceseracciu, Monica Reggiani, Claudio Pizzolato

project(TrialData)

# List source files
set(TrialData_HEADERS ceinms/DataTable.h
                      ceinms/TrialData.h
                      )

set(TrialData_TEMPLATE_IMPLEMENTATION
                      ceinms/DataTable.cpp)

set_source_files_properties( ${TrialData_TEMPLATE_IMPLEMENTATION} PROPERTIES HEADER_FILE_ONLY TRUE)

source_group("Header Files" FILES ${TrialData_HEADERS})
source_group("Source Files" FILES ${TrialData_TEMPLATE_IMPLEMENTATION})

add_library(TrialData INTERFACE)
target_include_directories(TrialData INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
                                        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

add_custom_target(TrialData_IDE SOURCES ${TrialData_HEADERS} ${TrialData_TEMPLATE_IMPLEMENTATION})

#Install
install(FILES ${TrialData_HEADERS} ${TrialData_TEMPLATE_IMPLEMENTATION} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/ceinms COMPONENT Development)
install(TARGETS TrialData
        EXPORT CEINMSTargets)
