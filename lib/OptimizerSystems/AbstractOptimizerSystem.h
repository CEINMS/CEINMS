#ifndef CEINMS_Optimizers_AbstractOptimizerSystem_h
#define CEINMS_Optimizers_AbstractOptimizerSystem_h
#include <vector>

namespace CEINMS {
    namespace Optimizers {

        class AbstractOptimizerSystem {
        public:
            virtual ~AbstractOptimizerSystem() {}
            virtual void evaluate() = 0;
            virtual void setParameters(const std::vector<double>& x) = 0;
            virtual std::vector<double> getParameters() const = 0;
            virtual double getF() const = 0;
            virtual std::size_t getNoParameters() const = 0;
        };
    }
}

#endif
