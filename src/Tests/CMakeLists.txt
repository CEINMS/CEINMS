if(BUILD_TESTING)
    # Working
    add_subdirectory(Execution) 
    add_subdirectory(Barrier)
    add_subdirectory(Curve)
    add_subdirectory(DataTable)
    add_subdirectory(GenericCalibrationObjectiveFunction)   # Need to fix the TorqueErrorNormalised test
    add_subdirectory(GenericHybridFunction)
    add_subdirectory(JointContactModel)
    add_subdirectory(KneeBalanceContactModel)
    add_subdirectory(Mapper)
    add_subdirectory(MtuStepResponse)
    add_subdirectory(Optimizers)
    add_subdirectory(xmlInput)

    # Need to be written     
    # add_subdirectory(BatchEvaluator)                      # needs to be written
    # add_subdirectory(OptimizationSystem)                  # needs to be written 

    # Not working/causes error 
    # add_subdirectory(InverseDynamics)                     # error when running

    # Depends on something removed
    # add_subdirectory(MinTorqueErrorAndContactForces)      # requires OptimizaterSystems/MinTorqueErrorAndContactForces
    # add_subdirectory(LmtMaFromMcbs)                       # requires mcbs
    # add_subdirectory(OutputConnectors)                    # requires websocket
    # add_subdirectory(SurrogateModels)                     # requires rtb/plsr
endif()
