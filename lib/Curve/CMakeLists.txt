# Author: Elena Ceseracciu

# List source files
set(Curve_HEADERS   ceinms/Curve.h
                    ceinms/CircularVector.h)
set(Curve_TEMPLATE_IMPLEMENTATION
                    ceinms/Curve.cpp
                    ceinms/CircularVector.cpp)

set_source_files_properties( ${Curve_TEMPLATE_IMPLEMENTATION} PROPERTIES HEADER_FILE_ONLY TRUE)

add_library(Curve INTERFACE)
add_custom_target(Curve_IDE SOURCES ${Curve_HEADERS} ${Curve_TEMPLATE_IMPLEMENTATION})
target_include_directories(Curve INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
                                        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
# Install
install(TARGETS Curve
        EXPORT CEINMSTargets
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})

install(FILES ${Curve_HEADERS} ${Curve_TEMPLATE_IMPLEMENTATION} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/ceinms COMPONENT Development)
