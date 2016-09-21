namespace ceinms {

    template <typename NMSmodelT>
    NMSmodelFrameOptimisation<NMSmodelT>::NMSmodelFrameOptimisation(NMSmodelT& subject, const EmgInformedObjectiveFunctionSetup& setup) :
        subject_(subject),
        objFun_(subject, setup),
        parameterInterpreter_(subject, setup),
        emgInformed_(subject, objFun_, setup),
        simTkAdapter_(emgInformed_)
    {
        std::vector<double> up, lo;
        parameterInterpreter_.getUpperLowerBounds(up, lo);
        simTkAdapter_.setParameterLimits(ArrayConverter::toSimTKVector(lo), ArrayConverter::toSimTKVector(up));
    }


    template <typename NMSmodelT>
    void NMSmodelFrameOptimisation<NMSmodelT>::setExternalTorques(const std::vector<double>& externalTorques) {

        emgInformed_.setExternalTorques(externalTorques);
    }


    template <typename NMSmodelT>
    void NMSmodelFrameOptimisation<NMSmodelT>::optimise() {

      

        std::vector<double> measuredExcitations;
        subject_.getEmgs(measuredExcitations);
        emgInformed_.setMeasuredExcitations(measuredExcitations);

       // std::vector<double> pastEMGData;
       // subject_.getPastEmgs(pastEMGData);
        

        try {
            SimTK::Optimizer opt(simTkAdapter_);

            opt.setConvergenceTolerance(.00001);
            opt.useNumericalGradient(true);
            opt.useNumericalJacobian(true);
            
            /* compute  optimization */
            auto startingValues(ArrayConverter::toSimTKVector(parameterInterpreter_.getStartingParameters()));
            opt.optimize(startingValues); //starting values
        }
        catch (const std::exception& e) {
            std::cout << "NMSmodelFrameOptimisation.cpp: Caught exception" << std::endl;
            std::cout << e.what() << std::endl;
        }

        //finalise after optimization
   //     subject_.updateState_END_OF_HYBRID_MINIMIZATION();
    }
}
    
    
