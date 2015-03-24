#ifndef Ceinms_MinTorqueError_h
#define Ceinms_MinTorqueError_h
#include <vector>
#include <limits>
#include "TrialData.h"
#include "Result.h"
namespace CEINMS {

    class MinTorqueError {
    public:
        MinTorqueError() : f_(std::numeric_limits<double>::max()) {}
        void setDofsToCalibrate(const std::vector<std::string>& dofsToCalibrate);
        void setTrials(const std::vector<TrialData>& trials);
        void calculate(const std::vector<Result>& results); 
        double getValue() const { return f_; }
    private:
        void computeVariance();
        void updDofsToCalibrateIdx();
        double f_;
        std::vector<TrialData> trials_;
        std::vector<std::vector<double>> torqueVariance_;
        std::vector<std::string> dofsToCalibrate_;
        std::vector<std::vector<unsigned>> dofsToCalibrateIdx_;

    };
}

#endif