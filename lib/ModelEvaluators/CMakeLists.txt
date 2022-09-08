# Authors: Elena Ceseracciu, Monica Reggiani, Claudio Pizzolato

project(ModelEvaluators)

# List source files
set(ModelEvaluators_HEADERS 
    ceinms/ModelEvaluationBase.h
    ceinms/ModelEvaluationHybrid.h
    ceinms/ModelEvaluationOffline.h
    ceinms/ModelEvaluationOnline.h
)
set(ModelEvaluators_TEMPLATE_IMPLEMENTATION
    ModelEvaluationBase.cpp
    ModelEvaluationHybrid.cpp
    ModelEvaluationOffline.cpp
    ModelEvaluationOnline.cpp
)
                            
set(ModelEvaluators_SOURCES)

set_source_files_properties( ${ModelEvaluators_TEMPLATE_IMPLEMENTATION} PROPERTIES HEADER_FILE_ONLY TRUE)

source_group("Header Files" FILES ${ModelEvaluators_HEADERS})
source_group("Source Files" FILES ${ModelEvaluators_TEMPLATE_IMPLEMENTATION} ${ModelEvaluators_SOURCES})

add_library(ModelEvaluators STATIC ${ModelEvaluators_HEADERS} ${ModelEvaluators_TEMPLATE_IMPLEMENTATION} ${ModelEvaluators_SOURCES})

# Save path to include directory for "internal" usage
set(ModelEvaluators_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}" ceinms CACHE INTERNAL "Include directory for ModelEvaluators library")

# Install
install(FILES ${ModelEvaluators_HEADERS} ${ModelEvaluators_TEMPLATE_IMPLEMENTATION} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/ceinms  COMPONENT Development)

set_target_properties(ModelEvaluators PROPERTIES FOLDER "Libraries")