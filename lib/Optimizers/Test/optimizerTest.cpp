#include "SimulatedAnnealing.h"
#include "AbstractOptimizerSystem.h"
#include "SimulatedAnnealingParameters.h"
#include <vector>
using std::vector;
#include <iostream>
using std::cout;
using std::endl;
using namespace CEINMS::Optimizers;

class RosenbrockFunction : public AbstractOptimizerSystem {

public:
    RosenbrockFunction(double a, double b);
    ~RosenbrockFunction() {}
    void evaluate(const std::vector<double>& parameters);
    void setStartingParameterSet(const std::vector<double>& x) { startingParameters_ = x; }
    std::vector<double> getStartingParameterSet() const{ return startingParameters_; }
    double getF() const { return f_; }
    unsigned getNoParameters() const { return startingParameters_.size(); }
    void getUpperLowerBounds(std::vector<double>& upperBounds, std::vector<double>& lowerBounds) const {
        upperBounds = upperBounds_;
        lowerBounds = lowerBounds_;
    }

private:
    std::vector<double> startingParameters_;
    double a_, b_, f_;
    std::vector<double> upperBounds_, lowerBounds_;
};

RosenbrockFunction::RosenbrockFunction(double a, double b):
a_(a), b_(b), upperBounds_({ 10, 10 }), lowerBounds_({ -10, -10 }) { }

void RosenbrockFunction::evaluate(const std::vector<double>& parameters) {
    double x = parameters.at(0);
    double y = parameters.at(1);
    cout << "f(" << x << ", " << y << ") = ";
    f_ = (a_ - x)*(a_ - x) + b_*(y - x*x)*(y - x*x);
    cout << f_ << endl;
}


int main() {

    cout << "Testing Simulated Annealing with Rosenbrock function" << endl;
    RosenbrockFunction myFunction(1, 100);
    vector<double> startParameters({ 20, 20 });
    myFunction.setStartingParameterSet(startParameters);
    SimulatedAnnealingParameters annealingParameters;
    annealingParameters.epsilon = 1e-6;
    annealingParameters.noEpsilon = 4;
    annealingParameters.maxNoEval = 200000;
    annealingParameters.NS = 10;
    annealingParameters.NT = 20;
    annealingParameters.rt = 0.1;
    annealingParameters.T = 50000;
    SimulatedAnnealing<RosenbrockFunction> annealing(myFunction, annealingParameters);
    cout << "Start Optimizing" << endl;
    annealing.optimize();
    auto x(annealing.getOptimizedParameters());
   // vector<double> x({ 1, 1 });
   // myFunction.evaluate(x);
    cout << "Found minimum: f(" << x.at(0) << ", " << x.at(1) << ") = " << myFunction.getF() << endl;
    

}