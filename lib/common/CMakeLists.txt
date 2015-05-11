## Authors: Monica Reggiani, Claudio Pizzolato, Elena Ceseracciu

# List source files
set(Common_HEADERS  ceinms/ArrayConverter.h
                    ceinms/FileUtils.h
                    ceinms/Parameter.h
                    ceinms/SimulatedAnnealingParameters.h
                    ceinms/QueueData.h
                    ceinms/TimeCompare.h)

set(Common_SOURCES  Parameter.cpp
                    FileUtils.cpp)

source_group("Headers" FILES ${Common_HEADERS})
source_group("Sources" FILES ${Common_SOURCES})

# Define target
add_library(ceinms_common STATIC ${Common_SOURCES} ${Common_HEADERS})
target_link_libraries(ceinms_common LINK_PRIVATE ${Boost_FILESYSTEM_LIBRARY} ${Boost_SYSTEM_LIBRARY})
target_include_directories(ceinms_common PUBLIC ${Boost_INCLUDE_DIRS} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# Save path to include directory for "internal" usage
set(ceinms_common_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}" CACHE INTERNAL "Include directory for library of common files")

# Install
install(FILES ${Common_HEADERS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/ceinms)
install(TARGETS ceinms_common
        EXPORT CEINMSTargets
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        #INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}# only available from CMake 2.8.12
        )
