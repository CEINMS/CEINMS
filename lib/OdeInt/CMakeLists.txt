# Author: Elena Ceseracciu
set(OdeInt_HEADERS  ceinms/OdeInt.h
                    ceinms/Output.h
                    ceinms/StepperBase.h
                    ceinms/StepperDopr5.h)

add_library(OdeInt INTERFACE)
add_custom_target(OdeInt_IDE SOURCES ${OdeInt_HEADERS})
target_include_directories(OdeInt INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)
