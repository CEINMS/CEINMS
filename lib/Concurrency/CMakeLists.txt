#Author: Elena Ceseracciu

set(Concurrency_HEADERS Latch.h
                        Queue.h)

set(Concurrency_TEMPLATE_IMPLEMENTATIONS Queue.cpp)
set_source_files_properties(${Concurrency_TEMPLATE_IMPLEMENTATIONS} PROPERTIES HEADER_FILE_ONLY TRUE)

set(Concurrency_SOURCES Latch.cpp)

source_group("Header files" FILES ${Concurrency_HEADERS})
source_group("Source files" FILES ${Concurrency_TEMPLATE_IMPLEMENTATIONS} ${Concurrency_SOURCES})

include_directories(include)
add_library(Concurrency ${Concurrency_HEADERS} ${Concurrency_TEMPLATE_IMPLEMENTATIONS} ${Concurrency_SOURCES} )

set(Concurrency_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}" CACHE PATH "Include directory for Concurrency library" FORCE)
mark_as_advanced(Concurrency_INCLUDE_DIRS)

install(FILES ${Concurrency_HEADERS} ${Concurrency_TEMPLATE_IMPLEMENTATIONS}
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(TARGETS Concurrency
        EXPORT CEINMSTargets
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
