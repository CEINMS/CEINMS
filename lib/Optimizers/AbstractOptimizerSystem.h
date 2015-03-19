#ifndef CEINMS_Optimizers_AbstractOptimizerSystem_h
#define CEINMS_Optimizers_AbstractOptimizerSystem_h
#include <vector>

namespace CEINMS {
    namespace Optimizers {

        class AbstractOptimizerSystem {
        public:
            virtual ~AbstractOptimizerSystem() {}
            virtual void evaluate(const std::vector<double>& x) = 0;
            virtual void setStartingParameterSet(const std::vector<double>& x) = 0;
            virtual std::vector<double> getStartingParameterSet() const = 0;
            virtual double getF() const = 0;
            virtual unsigned getNoParameters() const = 0;
        };
    }
}

#endif