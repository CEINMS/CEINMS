include_directories(
	${OPENSIMSIMBODY_INCLUDE_DIRS}
	${mcbs_INCLUDE_DIRS}
	#${MR3tools_INCLUDE_DIRS}
	${Plsr_INCLUDE_DIRS}
	${InputConnectors_INCLUDE_DIRS}
	${OutputConnectors_INCLUDE_DIRS}
	${NMSmodel_INCLUDE_DIRS}
	${Curve_INCLUDE_DIRS}
	${Concurrency_INCLUDE_DIRS}
	${xmlInput_INCLUDE_DIRS}
	${FileIO_INCLUDE_DIRS}
	${CMAKE_SOURCE_DIR}/lib/Hybrid
	${ceinms_common_INCLUDE_DIRS}
	${Optimizers_INCLUDE_DIRS}
	${OptimizationSystems_INCLUDE_DIRS}
	${TrialData_INCLUDE_DIRS}
	${JointContactModels_INCLUDE_DIRS}
	${ModelEvaluators_INCLUDE_DIRS}
	${RTOSIM_INCLUDE_DIRS}
	${OPENSIM_INCLUDE_DIRS}       
	# ${SIMTK_INCLUDE_DIRS}
 )

set(itrain_HEADERS 
                   ../Logger.h 
                   ../SimpleFileLogger.h 
                   ../StorageLogger.h 
                   ../SimulationManager.h)
                   
 
                   
add_executable(itrain itrain.cpp ${itrain_HEADERS})

target_link_libraries(itrain
    FileIO
    xmlInput
    NMSmodel
    InputConnectors
    OutputConnectors
#	JointContactModels
    OptimizationSystems
    ${Boost_PROGRAM_OPTIONS_LIBRARY}
    ${Boost_FILESYSTEM_LIBRARY}
#  rtb::Common 
	mcbs::Spline 
	mcbs::SplineSetHelper
#   MR3tools 
	rtb::rtosim
	rtb::Filter 
	rtb::Plsr
)
