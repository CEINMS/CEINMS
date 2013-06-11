#ifndef Stepperbase_h
#define StepperBase_h

#include <vector>

class StepperBase {
public:
    StepperBase(std::vector<double>& y, std::vector<double>& dydx, double& x, const double aTol,
                const double rTol, bool dense) : x_(x), y_(y), dydx_(dydx), aTol_(aTol),
    rTol_(rTol), dense_(dense), n_(y.size()) ,nEqn_(y.size()), yOut_(y.size()),yErr_(y.size()) {}
    
    double& x_;
    double xOld_;
    std::vector<double>& y_;
    std::vector<double>& dydx_;
    double aTol_, rTol_;
    bool dense_;
    double hDid_;
    double hNext_;
    double EPS_;
    int n_, nEqn_;
    std::vector<double> yOut_, yErr_;
};

#endif