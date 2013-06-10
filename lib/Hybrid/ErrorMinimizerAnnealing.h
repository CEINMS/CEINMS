#ifndef ErrorMinimizerAnnealing_h
#define ErrorMinimizerAnnealing_h

#include "HybridWeightings.h"

namespace Hybrid {
    
    template<typename NMSmodelT>
    class ErrorMinimizerAnnealing {
    
    public:
        ErrorMinimizerAnnealing(NMSmodelT& subject);  
        void setMusclesNamesWithEmgToTrack(const std::vector<std::string>& musclesNamesWithEmgToTrack);
        void setMusclesNamesWithEmgToPredict(const std::vector<std::string>& musclesNamesWithEmgToPredict);
        void setSingleExternalTorque(double externalTorque, const std::string& whichDof);
        void setAllExternalTorques(std::vector<double> externalTorques, const std::vector< std::string >& dofs);
        void setTime(double time) {currentTime_ = time;}
        void setWeightings(HybridWeightings weightings) {weightings_ = weightings;}
        void minimize();
    private:
        HybridWeightings weightings_;
        NMSmodelT& subject_;
        std::vector<double> currentExternalTorques_;
        std::vector<std::string> subjectDofNames_;
        std::vector<std::string> currentDofNames_;
        std::vector<std::string> musclesNamesWithEmgToTrack_;
        std::vector<std::string> musclesNamesWithEmgToPredict_;
        std::vector<double> lastActivations_;
        double currentTime_;
    };
}

#include "ErrorMinimizerAnnealing.cpp"

#endif