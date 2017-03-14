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

add_library(Optimizers INTERFACE)
add_custom_target(Optimizers_IDE SOURCES ${Optimizers_HEADERS} ${Optimizers_TEMPLATE_IMPLEMENTATION})
target_link_libraries(Optimizers INTERFACE ceinms_common)
target_include_directories( Optimizers INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
                                              $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

install(FILES ${Optimizers_HEADERS} ${Optimizers_TEMPLATE_IMPLEMENTATION} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/ceinms COMPONENT Development)
install(TARGETS Optimizers
        EXPORT CEINMSTargets)
