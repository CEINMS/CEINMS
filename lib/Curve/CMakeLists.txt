# Author: Elena Ceseracciu

# List source files
set(Curve_HEADERS   ceinms/Curve.h
                    ceinms/CircularVector.h)
set(Curve_TEMPLATE_IMPLEMENTATION
                    ceinms/Curve.cpp
                    ceinms/CircularVector.cpp)

set_source_files_properties( ${Curve_TEMPLATE_IMPLEMENTATION} PROPERTIES HEADER_FILE_ONLY TRUE)

# Save path to include directory for "internal" usage
set(Curve_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}" CACHE INTERNAL "Include directory for Curve library")

if (NOT CMAKE_VERSION VERSION_LESS 3.0.0)
     add_library(Curve INTERFACE)
     install(TARGETS Curve
        EXPORT CEINMSTargets
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
endif()

# Install
install(FILES ${Curve_HEADERS} ${Curve_TEMPLATE_IMPLEMENTATION} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/ceinms)
