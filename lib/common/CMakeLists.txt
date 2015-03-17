## Authors: Monica Reggiani, Claudio Pizzolato, Elena Ceseracciu

# List source files
set(Common_HEADERS  ArrayConverter.h
                    FileUtils.h
                    Parameter.h
                    SimulatedAnnealingParameters.h
                    QueueData.h
					TimeCompare.h)

set(Common_SOURCES  Parameter.cpp
                    FileUtils.cpp)

source_group("Headers" FILES ${Common_HEADERS})
source_group("Sources" FILES ${Common_SOURCES})

# Define target
add_library(ceinms_common STATIC ${Common_SOURCES} ${Common_HEADERS})
target_link_libraries(ceinms_common LINK_PRIVATE ${Boost_FILESYSTEM_LIBRARY} ${Boost_SYSTEM_LIBRARY})
target_include_directories(ceinms_common PUBLIC ${Boost_INCLUDE_DIRS})

# Save path to include directory for "internal" usage
set(ceinms_common_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR} CACHE STRING "Include directory for library of common files")
mark_as_advanced(ceinms_common_INCLUDE_DIRS)

# Install
install(FILES ${Common_HEADERS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}) #should add namespace
install(TARGETS ceinms_common
        EXPORT CEINMSTargets
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        #INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}# only available from CMake 2.8.12
        )
