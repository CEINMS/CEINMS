/* -------------------------------------------------------------------------- *
 * CEINMS is a standalone toolbox for neuromusculoskeletal modelling and      *
 * simulation. CEINMS can also be used as a plugin for OpenSim either         *
 * through the OpenSim GUI or API. See https://simtk.org/home/ceinms and the  *
 * NOTICE file for more information. CEINMS development was coordinated       *
 * through Griffith University and supported by the Australian National       *
 * Health and Medical Research Council (NHMRC), the US National Institutes of *
 * Health (NIH), and the European Union Framework Programme 7 (EU FP7). Also  *
 * see the PROJECTS file for more information about the funding projects.     *
 *                                                                            *
 * Copyright (c) 2010-2015 Griffith University and the Contributors           *
 *                                                                            *
 * CEINMS Contributors: C. Pizzolato, M. Reggiani, M. Sartori,                *
 *                      E. Ceseracciu, and D.G. Lloyd                         *
 *                                                                            *
 * Author(s): C. Pizzolato                                                    *
 *                                                                            *
 * CEINMS is licensed under the Apache License, Version 2.0 (the "License").  *
 * You may not use this file except in compliance with the License. You may   *
 * obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0.*
 *                                                                            *
 * Unless required by applicable law or agreed to in writing, software        *
 * distributed under the License is distributed on an "AS IS" BASIS,          *
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.   *
 * See the License for the specific language governing permissions and        *
 * limitations under the License.                                             *
 * -------------------------------------------------------------------------- */

#include "ceinms/SimulatedAnnealing.h"
#include "ceinms/AbstractOptimizerSystem.h"
#include "ceinms/SimulatedAnnealingParameters.h"
#include <vector>
using std::vector;
#include <iostream>
using std::cout;
using std::endl;
using ceinms::Optimizers::AbstractOptimizerSystem;
using ceinms::Optimizers::SimulatedAnnealing;

class RosenbrockFunction : public AbstractOptimizerSystem {

public:
    RosenbrockFunction(double a, double b);
    ~RosenbrockFunction() {}
    void evaluate();
    void setParameters(const std::vector<double>& x) { parameters_ = x; }
    std::vector<double> getParameters() const{ return parameters_; }
    double getF() const { return f_; }
    size_t getNoParameters() const { return parameters_.size(); }
    void getUpperLowerBounds(std::vector<double>& upperBounds, std::vector<double>& lowerBounds) const {
        upperBounds = upperBounds_;
        lowerBounds = lowerBounds_;
    }

private:
    std::vector<double> parameters_;
    double a_, b_, f_;
    std::vector<double> upperBounds_, lowerBounds_;
};

RosenbrockFunction::RosenbrockFunction(double a, double b):
a_(a), b_(b), upperBounds_({ 10, 10 }), lowerBounds_({ -10, -10 }) { }

void RosenbrockFunction::evaluate() {
    double x = parameters_.at(0);
    double y = parameters_.at(1);
    cout << "f(" << x << ", " << y << ") = ";
    f_ = (a_ - x)*(a_ - x) + b_*(y - x*x)*(y - x*x);
    cout << f_ << endl;
}


int main() {

    cout << "Testing Simulated Annealing with Rosenbrock function" << endl;
    RosenbrockFunction myFunction(1, 100);
    vector<double> startParameters({ 20, 20 });
    myFunction.setParameters(startParameters);
    ceinms::SimulatedAnnealingParameters annealingParameters;
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
