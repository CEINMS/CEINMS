# Authors: Elena Ceseracciu, Monica Reggiani, Claudio Pizzolato

project(Optimizers)

include_directories(
  ${ceinms_common_INCLUDE_DIRS}
)
# List source files
set(Optimizers_HEADERS AbstractOptimizerSystem.h
                                     AbstractOptimizer.h
                                     SimulatedAnnealing.h
                        )
set(Optimizers_SOURCES )

set(Optimizers_TEMPLATE_IMPLEMENTATION SimulatedAnnealing.cpp)

set_source_files_properties( ${Optimizers_TEMPLATE_IMPLEMENTATION} PROPERTIES HEADER_FILE_ONLY TRUE)

source_group("Header Files" FILES ${Optimizers_HEADERS})
source_group("Source Files" FILES ${Optimizers_SOURCES} ${Optimizers_TEMPLATE_IMPLEMENTATION})

add_library(Optimizers STATIC ${Optimizers_SOURCES} ${Optimizers_HEADERS} ${Optimizers_TEMPLATE_IMPLEMENTATION})

# target_link_libraries(Calibration xmlInput FileIO)
target_include_directories( Optimizers
                             PRIVATE #${NMSmodel_INCLUDE_DIRS}
                                    # ${Curve_INCLUDE_DIRS}
                                    # ${FileIO_INCLUDE_DIRS}
                                     ${ceinms_common_INCLUDE_DIRS})

# Save path to include directory for "internal" usage
set(Optimizers_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}  ${ceinms_common_INCLUDE_DIRS} CACHE STRING "Include directories for Optimizers library")
mark_as_advanced(Optimizers_INCLUDE_DIRS)



add_subdirectory(Test)
