# - Try to find OpenSim and SimTK
# Once done this will define
#  OPENSIM_FOUND - System has OpenSim
#  OPENSIM_INCLUDE_DIRS - The OpenSim include directories
#  SIMTK_INCLUDE_DIRS - The SimTK include directories
#  OPENSIM_LIBRARIES - The libraries needed to use Opensim
#  OPENSIM_DEFINITIONS - Compiler switches required for using OpenSim


set(OPENSIM_POSSIBLE_DIRECTORIES 
	~/Programs/opensim
	$ENV{OPENSIM_HOME}
	"C:/OpenSim\ 3.0/"
	"C:/OpenSim3.1/"
	)

find_path(OPENSIM_INCLUDE_DIRS OpenSim/OpenSim.h
          PATHS ${OPENSIM_POSSIBLE_DIRECTORIES}
          PATH_SUFFIXES sdk/include )
MESSAGE( STATUS "OpenSim Include path: ${OPENSIM_INCLUDE_DIRS}")

find_path(SIMTK_INCLUDE_DIRS Simbody.h
          PATHS ${OPENSIM_POSSIBLE_DIRECTORIES}
          PATH_SUFFIXES sdk/include/SimTK/include sdk/include/SimTK/simbody)
MESSAGE( STATUS "SimTK Include path: ${SIMTK_INCLUDE_DIRS}")
		  
#find_path(OPENSIM_LIBS_DIR
#		  NAMES
#			  osimCommon.lib
#			  osimCommon.so
#			  libosimCommon.so
#          PATHS ${OPENSIM_POSSIBLE_DIRECTORIES}
#          PATH_SUFFIXES sdk/lib lib)
#MESSAGE( STATUS "OpenSim Lib path: ${OPENSIM_LIBS_DIR}")
		  
#why?
find_path(OPENSIM_DLL_DIR 
		  NAMES
			  osimCommon.dll
#			  id
          PATHS ${OPENSIM_POSSIBLE_DIRECTORIES}
          PATH_SUFFIXES bin)
MESSAGE( STATUS "OpenSim dll path: ${OPENSIM_DLL_DIR}")	  

#SET(OPENSIM_LIBRARY_DIRS ${OPENSIM_LIBS_DIR} ${OPENSIM_DLL_DIR})
#MESSAGE( STATUS "OpenSim Library path: ${OPENSIM_LIBRARY_DIRS}")

if(UNIX)
    set(NameSpace )
else()
    set(NameSpace OpenSim_)
endif()

find_library(osimSimulation
             NAMES osimSimulation
             PATHS ${OPENSIM_POSSIBLE_DIRECTORIES}
             PATH_SUFFIXES sdk/lib lib)

find_library(osimCommon
             NAMES osimCommon
             PATHS ${OPENSIM_POSSIBLE_DIRECTORIES}
             PATH_SUFFIXES sdk/lib lib)

find_library(osimActuators
             NAMES osimActuators
             PATHS ${OPENSIM_POSSIBLE_DIRECTORIES}
             PATH_SUFFIXES sdk/lib lib)

find_library(osimAnalyses
             NAMES osimAnalyses
             PATHS ${OPENSIM_POSSIBLE_DIRECTORIES}
             PATH_SUFFIXES sdk/lib lib)

find_library(osimTools
             NAMES osimTools
             PATHS ${OPENSIM_POSSIBLE_DIRECTORIES}
             PATH_SUFFIXES sdk/lib lib)

find_library(${NameSpace}SimTKcommon
             NAMES ${NameSpace}SimTKcommon
             PATHS ${OPENSIM_POSSIBLE_DIRECTORIES}
             PATH_SUFFIXES sdk/lib lib)

find_library(${NameSpace}SimTKmath
             NAMES ${NameSpace}SimTKmath
             PATHS ${OPENSIM_POSSIBLE_DIRECTORIES}
             PATH_SUFFIXES sdk/lib lib)

find_library(${NameSpace}SimTKsimbody
             NAMES ${NameSpace}SimTKsimbody
             PATHS ${OPENSIM_POSSIBLE_DIRECTORIES}
             PATH_SUFFIXES sdk/lib lib)
#does this exist?
#find_library(SimTKlapack
#             NAMES SimTKlapack
#             PATHS ${OPENSIM_POSSIBLE_DIRECTORIES}
#             PATH_SUFFIXES sdk/lib lib)

set(OPENSIM_LIBRARIES ${osimSimulation} ${osimCommon} ${osimActuators} ${osimAnalyses} ${osimTools} ${${NameSpace}SimTKcommon} ${${NameSpace}SimTKmath} ${${NameSpace}SimTKsimbody})


#IF(WIN32)
#	SET(PLATFORM_LIBS  pthreadVC2)
#ELSE (WIN32)
#  SET(NameSpace "")
#  IF(APPLE)
#	SET(PLATFORM_LIBS  SimTKAtlas)
#  ELSE(APPLE)
#	SET(PLATFORM_LIBS SimTKAtlas_Lin_generic)
#  ENDIF(APPLE)
#ENDIF (WIN32)

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set OPENSIM_FOUND to TRUE
# if all listed variables are TRUE
# DEFAULT_MSG is predefined... change only if you need a custom msg
find_package_handle_standard_args(OpenSim DEFAULT_MSG
                                  OPENSIM_LIBRARIES OPENSIM_INCLUDE_DIRS SIMTK_INCLUDE_DIRS)

# Mark the following variables as advanced (they are not displayed in any 
# of the cmake GUIs unless the show advanced option is on.
#mark_as_advanced(LIBXML2_INCLUDE_DIR LIBXML2_LIBRARY )


