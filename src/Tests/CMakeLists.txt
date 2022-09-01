if(BUILD_TESTING)
    # Working
    add_subdirectory(JointContactModel)                     # add more info/explanation
    add_subdirectory(Barrier)                               # add more info/explanation
    add_subdirectory(Curve)   
    add_subdirectory(DataTable)                             # add more info/explanation
    add_subdirectory(GenericCalibrationObjectiveFunction)   # add more info/explanation
    add_subdirectory(GenericHybridFunction)                 # add more info/explanation
    add_subdirectory(KneeBalanceContactModel)               # add more info/explanation
    add_subdirectory(Mapper)                                # add more info/explanation
    add_subdirectory(MtuStepResponse)                       # add more info/explanation
    add_subdirectory(Optimizers)                            # add more info/explanation
    add_subdirectory(xmlInput)                              # add more info/explanation

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
