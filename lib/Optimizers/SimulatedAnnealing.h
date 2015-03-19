#ifndef CEINMS_Optimizers_Simulatedannealing_h
#define CEINMS_Optimizers_Simulatedannealing_h

#include "AbstractOptimizer.h"
#include "SimulatedAnnealingParameters.h"
#include <vector>

namespace CEINMS {
    namespace Optimizers {

        template<typename OptimizerSystem>
        class SimulatedAnnealing : public AbstractOptimizer  {
        public:
            SimulatedAnnealing(OptimizerSystem& optimizerSystem, SimulatedAnnealingParameters simulatedAnnealingParameters);
            ~SimulatedAnnealing() {};
            bool optimize();
            std::vector<double> getOptimizedParameters() const { return xOpt_; }

        private:
            void checkBounds(int k);
            double computeMetropolisCriteria(double t) const;
            bool doTerminate();
            void updateFandFlatest();

            OptimizerSystem& optimizerSystem_;

            std::vector<double> x_;
            std::vector<double> upperBounds_;
            std::vector<double> lowerBounds_;
            std::vector<double> xOpt_;
            std::vector<double> xp_;
            std::vector<double> v_;
            std::vector<double> fLatest_;
            std::vector<int>    noAccepted_;
            int                 noParameters_;
            double              f_, fp_, fOpt_;
            //annealing parameters    
            unsigned                nt_;
            unsigned                ns_;
            double                  rt_;
            double                  t_;
            unsigned                maxNoEval_;
            double                  epsilon_;
            unsigned                nEpsilon_;
        };


    }
}
#include "SimulatedAnnealing.cpp"
#endif