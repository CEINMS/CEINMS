#ifndef ceinms_Hybrid_Parameters_RecursiveEMGs_h
#define ceinms_Hybrid_Parameters_RecursiveEMGs_h

#include "NMSmodel.h"

#include <vector>
#include <string>

namespace ceinms {
    namespace Hybrid {
        namespace Parameters {
            template<typename NMSmodelT>
            class RecursiveEMGs;

            template<typename NMSmodelT>
            std::ostream& operator<< (std::ostream& output,
                const RecursiveEMGs<NMSmodelT>& p);


            template<typename NMSmodelT>
            class RecursiveEMGs{

            public:
                RecursiveEMGs(NMSmodelT& subject,
                    const std::vector< std::string >& muscleNamesWithEMGtoTrack,
                    const std::vector< std::string >& muscleNamesWithEMGtoPredict);

                int getNoParameters() { return noParameters_; }
                void getStartingVectorParameters(std::vector<double>& x);
                void setVectorParameters(const std::vector<double>& x);
                void setUpperLowerBounds(std::vector<double>& upperBounds, std::vector<double>& lowerBounds);
                friend std::ostream& operator<< <> (std::ostream& output, const RecursiveEMGs& p);

            private:
                NMSmodelT& subject_;

                unsigned noParameters_;
                mutable std::vector<double> x_;
                std::vector<double> startingEmgValues_;
                std::vector<unsigned> muscleIndexWithEMGtoTrack_;
                std::vector<unsigned> muscleIndexWithEMGtoPredict_;
                std::vector<unsigned> muscleIndexWithEMGtoOptimize_;
            };
        }
    }
}

#include "RecursiveEMGs.cpp"

#endif