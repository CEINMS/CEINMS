#ifndef CEINMS_Optimizers_AbstractObjectiveFunction_h
#define CEINMS_Optimizers_AbstractObjectiveFunction
#include <vector>

namespace CEINMS {
    namespace Optimizers {

        class AbstractObjectiveFunction {
        public:
            virtual ~AbstractObjectiveFunction() {}
            virtual void evaluate(const std::vector<double>& x) = 0;
            virtual double getF() const = 0;
        };
    }
}

#endif