# Authors: Elena Ceseracciu, Monica Reggiani, Claudio Pizzolato

project(Optimizers)

# List source files
set(Optimizers_HEADERS  ceinms/AbstractOptimizer.h
                        ceinms/SimulatedAnnealing.h
                        )

set(Optimizers_TEMPLATE_IMPLEMENTATION ceinms/SimulatedAnnealing.cpp)

set_source_files_properties( ${Optimizers_TEMPLATE_IMPLEMENTATION} PROPERTIES HEADER_FILE_ONLY TRUE)

source_group("Header Files" FILES ${Optimizers_HEADERS})
source_group("Template implementation Files" FILES ${Optimizers_TEMPLATE_IMPLEMENTATION})

add_library(Optimizers STATIC ${Optimizers_HEADERS} ${Optimizers_TEMPLATE_IMPLEMENTATION})

# target_link_libraries(Calibration xmlInput FileIO)
target_include_directories( Optimizers
                             PRIVATE ceinms ${ceinms_common_INCLUDE_DIRS})

# Save path to include directory for "internal" usage
set(Optimizers_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}" ${ceinms_common_INCLUDE_DIRS} CACHE INTERNAL "Include directories for Optimizers library")

install(FILES ${Optimizers_HEADERS} ${Optimizers_TEMPLATE_IMPLEMENTATION} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/ceinms)



