#ifndef ceinms_Optimizers_AbstractOptimizer_h
#define ceinms_Optimizers_AbstractOptimizer_h
#include <vector>

namespace ceinms {
    namespace Optimizers {
        class AbstractOptimizer {
        public:
            virtual ~AbstractOptimizer() {};
            virtual bool optimize() = 0;
            virtual std::vector<double> getOptimizedParameters() const = 0;

        };
    }
}

#endif