#ifndef CEINMS_Optimizers_AbstractOptimizer_h
#define CEINMS_Optimizers_AbstractOptimizer_h
#include <vector>

namespace CEINMS {
    namespace Optimizers {
      
        class AbstractOptimizer {
        public:
            virtual ~AbstractOptimizer() {};
            virtual bool optimize() = 0;
            virtual std::vector<double> getOptimizedParameters() const  = 0;

        };
    }
}


#endif