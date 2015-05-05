#ifndef ceinms_Hybrid_StaticComputationMode_Default_h
#define ceinms_Hybrid_StaticComputationMode_Default_h

#include <vector>

namespace ceinms{
    namespace Hybrid {
        namespace StaticComputationMode {

            template<typename NMSmodelT>
            class Default {

            public:
                Default(NMSmodelT& subject);
                void getTorques(std::vector<double>& torques);
                void setTime(double time) { time_ = time; };
            private:
                NMSmodelT& subject_;
                void getMusclesToUpdate();
                unsigned nMuscles_;
                std::vector<double> emgDataT1_, forceDataT1_;
                std::vector<unsigned> musclesToUpdate_;
                double time_;
            };
        }
    }
}
#include "Default.cpp"

#endif