include_directories(
	${InputConnectors_INCLUDE_DIRS} 
	${ceinms_common_INCLUDE_DIRS}	
	${OPENSIM_INCLUDE_DIRS}       
    ${SIMTK_INCLUDE_DIRS}
	${Concurrency_INCLUDE_DIRS}
	${NMSmodel_INCLUDE_DIRS}
	${Curve_INCLUDE_DIRS}
	${xmlInput_INCLUDE_DIRS}
	${OutputConnectors_INCLUDE_DIRS}
	${osimSpliner_INCLUDE_DIRS}
	${mbcs_INCLUDE_DIRS}
	${FileIO_INCLUDE_DIRS}
)

add_definitions(/DBASE_DIR="${CMAKE_CURRENT_LIST_DIR}")

add_executable(Test_LmtMaFromMcbs lmtMaFromMcbsTest.cpp)
target_link_libraries(Test_LmtMaFromMcbs
	InputConnectors
	ceinms_common
	NMSmodel
	Concurrency
	xmlInput
	OutputConnectors
	SplineSetHelper
	mcbs::Spline
	${OPENSIM_LIBRARIES}
)

add_executable(Test_LmtMaFromMcbsAndOsim lmtMaFromMcbsAndOsimTest.cpp)
target_link_libraries(Test_LmtMaFromMcbsAndOsim
	InputConnectors
	ceinms_common
	NMSmodel
	Concurrency
	xmlInput
	OutputConnectors
	SplineSetHelper
	mcbs::Spline
	${OPENSIM_LIBRARIES}
)
#target_link_libraries(Test_LmtMaFromMcbs InputConnectors ceinms_common NMSmodel Concurrency xmlInput OutputConnectors SplineSetHelper mcbs::Spline debug ${OPENSIM_LIBRARIES_DEBUG})


set(N_DOFS 4)
#FIND_PROGRAM(osimSpliner_BIN NAMES "osimSpliner${N_DOFS}" PATHS ${osimSpliner_DIR}/../../bin NO_DEFAULT_PATH NO_CMAKE_SYSTEM_PATH NO_SYSTEM_ENVIRONMENT_PATH NO_CMAKE_FIND_ROOT_PATH)
#execute_process(COMMAND ${osimSpliner_BIN} 
#                TestData/Subject/scaled.osim 
#                TestData/SplinesSetup/dofAngles.cfg
#                TestData/SplinesSetup/muscles.in
#                TestData/GeneratedSplines/
#				RESULT_VARIABLE rv      
#				OUTPUT_VARIABLE ov    
#				WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
#)
#message("${ov}")
