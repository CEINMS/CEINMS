#ifndef CEINMS_Optimizers_Simulatedannealing_h
#define CEINMS_Optimizers_Simulatedannealing_h

#include "AbstractOptimizer.h"
#include "SimulatedAnnealingParameters.h"
#include <vector>

namespace CEINMS {
    namespace Optimizers {

        template<typename ObjectiveFunction>
        class SimulatedAnnealing : AbstractOptimizer  {
        public:
            SimulatedAnnealing(ObjectiveFunction& objectiveFunction, SimulatedAnnealingParameters simulatedAnnealingParameters);
            ~SimulatedAnnealing();
            bool optimize();
            std::vector<double> getOptimizedParameters();

        private:
            void checkBounds(int k);
            ObjectiveFunctionT objectiveFunction_;

            std::vector<double> x_;
            std::vector<double> upperBounds_;
            std::vector<double> lowerBounds_;
            std::vector<double> xOpt_;
            std::vector<double> xp_;
            std::vector<double> v_;
            std::vector<int>    noAccepted_;
            int                 noParameters_;

            //annealing parameters    
            double              nt_;
            double              ns_;
            double              rt_;
            double              t_;
            int                 maxNoEval_;
        };
    }
}


#endif