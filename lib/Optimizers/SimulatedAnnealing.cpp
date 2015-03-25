//__________________________________________________________________________
// Author(s): Claudio Pizzolato, Monica Reggiani - October 2013
// email:  claudio.pizzolato@griffithuni.edu.au
//         monica.reggiani@gmail.com
//
// DO NOT REDISTRIBUTE WITHOUT PERMISSION
//__________________________________________________________________________
//


#include <iostream>
using std::cout;
using std::endl;

#include <math.h>
#include <time.h>
#include <vector>
using std::vector;
#include <string>
using std::string;
#include <time.h>
#include <limits>

#define LOG_SIMULATED_ANNEALING

namespace CEINMS {
    namespace Optimizers {

        template<typename OptimizerSystem>
        SimulatedAnnealing<OptimizerSystem>::SimulatedAnnealing(OptimizerSystem& optimizerSystem, SimulatedAnnealingParameters simulatedAnnealingParameters) :
            optimizerSystem_(optimizerSystem),
            nt_(simulatedAnnealingParameters.NT),
            ns_(simulatedAnnealingParameters.NS),
            rt_(simulatedAnnealingParameters.rt),
            t_(simulatedAnnealingParameters.T),
            epsilon_(simulatedAnnealingParameters.epsilon),
            nEpsilon_(simulatedAnnealingParameters.noEpsilon),
            maxNoEval_(simulatedAnnealingParameters.maxNoEval),
            fLatest_(simulatedAnnealingParameters.noEpsilon, std::numeric_limits<double>::max()),
            f_(std::numeric_limits<double>::max()),
            fp_(std::numeric_limits<double>::max()),
            fOpt_(std::numeric_limits<double>::max())
            {

            noParameters_ = optimizerSystem_.getNoParameters();
            x_ = optimizerSystem_.getParameters();
            optimizerSystem_.getUpperLowerBounds(upperBounds_, lowerBounds_);

            xOpt_.resize(noParameters_);
            v_.resize(noParameters_);
            for (int i = 0; i < noParameters_; ++i)
                v_.at(i) = (upperBounds_.at(i) - lowerBounds_.at(i)) / 2;

            xp_.resize(noParameters_);
            noAccepted_.resize(noParameters_);

            // srand(time(NULL));
            srand(1);
        }


            template<typename OptimizerSystem>
        void SimulatedAnnealing<OptimizerSystem>::checkBounds(int k) {

            if ((xp_.at(k) < lowerBounds_.at(k)) ||
                (xp_.at(k) > upperBounds_.at(k)))
                xp_.at(k) = lowerBounds_.at(k) + ((upperBounds_.at(k) - lowerBounds_.at(k)) * rand() / RAND_MAX);
        }

        template<typename OptimizerSystem>
        bool SimulatedAnnealing<OptimizerSystem>::optimize() {

            int noEval = 0;
            for (int i = 0; i < noParameters_; ++i)
                noAccepted_.at(i) = 0;
            // DO UNTIL convergence or maxNoEvaluation
            bool terminate = false;
            while ((!terminate) && (noEval < maxNoEval_)) {

                // DO Nt times
                for (int i = 0; i < nt_; ++i) {

                    // DO Ns times
                    for (int j = 0; j < ns_; ++j) {

                        // DO i = 1, n
                        for (int k = 0; k < noParameters_; ++k) {
                            // x'_i = x_i + r v_i


                            xp_ = x_;

                            double factorForV = (2.*rand() / static_cast<double>(RAND_MAX)-1.);
                            xp_.at(k) = x_.at(k) + v_.at(k) * factorForV;
                            checkBounds(k);

#ifdef LOG_SIMULATED_ANNEALING
                            cout << "xp_ : ";
                            for (auto& val : xp_)
                                cout << val << " ";
                            cout << endl;
#endif

                            optimizerSystem_.setParameters(xp_);
                            optimizerSystem_.evaluate();
                            fp_ = optimizerSystem_.getF();
                            ++noEval;

#ifdef LOG_SIMULATED_ANNEALING
                            cout << "fp_ : " << fp_ << endl;
                            cout << "evalNo " << noEval << endl;
#endif       

                            // if f' < f then
                            if (fp_ < f_) {
                                // X = X'
                                x_ = xp_;
                                // f = f'
                                f_ = fp_;
                                // update statistics
                                noAccepted_.at(k)++;

                                // if f' < fopt then
                                //if (fp_ < fOpt_) {
                                if (fp_ < fOpt_) {
                                    // Xopt = X'
                                    xOpt_ = xp_;
                                    // fOpt = f'
                                    fOpt_ = fp_;
                                }
                            }
                            else { // IF f' > f THEN
                                double p = computeMetropolisCriteria(t_);
                                double randomValue = rand() / static_cast<double>(RAND_MAX);

                                if (randomValue < p) {

                                    x_ = xp_;             // X = X'
                                    f_ = fp_;
                                    noAccepted_.at(k)++;
                                }
                                else {

                                }

                            }

                        } // k

                    } // j

                    // Adjust V such that half of all trial are accepted

                    vector<double> c;
                    c.resize(noParameters_);
                    for (int h = 0; h < noParameters_; ++h)
                        c.at(h) = 2.;

                    for (int h = 0; h < noParameters_; ++h) {
                        double ratio = noAccepted_.at(h) / ns_;
                        if (ratio > 0.6)
                            v_.at(h) = v_.at(h) * (1 + c.at(h) * (ratio - 0.6) / 0.4);
                        else if (ratio < 0.4)
                            v_.at(h) = v_.at(h) / (1 + c.at(h) * ((0.4 - ratio) / 0.4));
                        if (v_.at(h) >(upperBounds_.at(h) - lowerBounds_.at(h)))
                            v_.at(h) = upperBounds_.at(h) - lowerBounds_.at(h);
                    }

                    for (int h = 0; h < noParameters_; ++h)
                        noAccepted_.at(h) = 0.;

                } // i

                // terminate?
                terminate = doTerminate();

                // reduce temperature
                t_ = t_ * rt_;
                updateFandFlatest();
                // restart
                x_ = xOpt_;
                
            }  /* end while */

            optimizerSystem_.setParameters(xOpt_);
            optimizerSystem_.evaluate();
//            cout << "total evaluations:" << noEval << endl;
            return 0;
        }

        template <typename OptimizerSystem>
        double SimulatedAnnealing<OptimizerSystem>::computeMetropolisCriteria(double t) const {
            double p = (f_ - fp_);
            return exp(p / t);
        }


        template <typename OptimizerSystem>
        bool SimulatedAnnealing<OptimizerSystem>::doTerminate() {
     
            fLatest_.at(0) = f_;

            for (unsigned int a = 0; a < nEpsilon_; ++a) {
                // we check that each DoF had a change in fOpt less than epsilon
                if (fabs(fLatest_.at(a) - fOpt_) > epsilon_) {
                    cout << "Return false" << endl;
                    cout << "fLatest_.at(" << a << ") - fOpt_ = "
                        << fLatest_.at(a) << " - " << fOpt_ << " > "
                        << epsilon_ << endl;
                    return false;
                }
            }
            cout << "Return true! " << endl;
            cout << "fOpt_ = " << fOpt_ << endl;
            return true;
        }



        template<typename OptimizerSystem>
        void SimulatedAnnealing<OptimizerSystem>::updateFandFlatest() {
            for (unsigned int a = 1; a < nEpsilon_; ++a)
                fLatest_.at(nEpsilon_ - a) = fLatest_.at(nEpsilon_ - a - 1);

            f_ = fOpt_;
        }

    }}