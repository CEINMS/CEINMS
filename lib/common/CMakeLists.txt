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
target_include_directories(ceinms_common PUBLIC ${Boost_INCLUDE_DIRS}
                                        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
                                        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

# Install
install(FILES ${Common_HEADERS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/ceinms COMPONENT Development)
install(TARGETS ceinms_common
        EXPORT CEINMSTargets
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT Applications
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT Development
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT Development)
