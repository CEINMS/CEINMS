#ifndef ceinms_Hybrid_StaticComputation_h
#define ceinms_Hybrid_StaticComputation_h

#include <vector>
#include "NMSmodel.h"

namespace ceinms {
    namespace Hybrid {

        template<typename NMSmodelT, typename StaticComputationMode>
        class StaticComputation;

        template<typename NMSmodelT, typename StaticComputationMode>
        std::ostream& operator<< (std::ostream& output, const StaticComputation<NMSmodelT, StaticComputationMode>& m);

        template<typename NMSmodelT, typename StaticComputationMode>
        class StaticComputation {

        public:
            StaticComputation(NMSmodelT& subject,
                const std::vector<std::string>& muscleNamesWithEMGtoTrack,
                const std::vector<std::string>& muscleNamesWithEMGtoPredict);
            void setTime(double time) { time_ = time; computationMode_.setTime(time); }
            void setExternalTorques(const std::vector<double>& externalTorques, const std::vector<std::string>& dofNames);
            void getExternalTorques(std::vector<double>& externalTorques) const;
            void getTorques(std::vector<double>& torques);
            void getInitialValuesOfTrackedEMGs(std::vector<double>& emgValues) const;
            void getAdjustedValuesOfTrackedEMGs(std::vector<double>& emgValues) const;
            void getCurrentEMGs(std::vector<double>& emgValues) const;

        private:

            NMSmodelT& subject_;
            StaticComputationMode computationMode_;
            double time_;
            std::vector<double> externalTorques_;
            std::vector<double> initialTorques_;   //torques at the start of the frame, before optimization
            std::vector<double> initialValueOfTrackedEMGs_;
            std::vector<unsigned> muscleIndexWithEMGtoTrack_;
            std::vector<unsigned> muscleIndexWithEMGtoPredict_;
            std::vector<unsigned> muscleIndexWithEMGtoOptimize_;


        };
    }
}
#include "StaticComputation.cpp"

#endif