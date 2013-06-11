#ifndef MinTorqueTrackEMGs_h
#define MinTorqueTrackEMGs_h

#include "HybridWeightings.h"

#include <vector>



namespace Hybrid {
    namespace ObjectiveFunction {
        
        template<typename StaticComputationT>
        class MinTorqueTrackEMGs {

        public:
            MinTorqueTrackEMGs( StaticComputationT& staticComputation,
                                    double epsilon, double nEpsilon, HybridWeightings hybridParameters);
			void   setEpsilon(double epsilon) { epsilon_ = epsilon;}
			void   setNoEpsilon(unsigned noEpsilon) { nEpsilon_ = noEpsilon;}
            void   evalFp();
            bool   isFacceptable();
            void   updateF();
            bool   isFoptAcceptable();
            void   updateFopt();
            bool   terminate();
            void   updateFandFlatest();
            void   printFs();
            double computeMetropolisCriteria(const double t);

        private:
            StaticComputationT& staticComputation_;

            double   fp_;
            double   f_;
            double   fOpt_;
            unsigned nEpsilon_;
            double   epsilon_;
            HybridWeightings hybridParameters_;

            std::vector<double>   fLatest_;
            std::vector<unsigned> dofsIndexListToCalibrate_;
        };
    }
}

#include "MinTorqueTrackEMGs.cpp"
#endif