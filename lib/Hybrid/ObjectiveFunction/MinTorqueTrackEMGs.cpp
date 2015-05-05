#include <float.h>
#include <iostream>
using std::cout;
using std::endl;
#include <vector>
using std::vector;

#include <math.h>
#include "HybridWeightings.h"

namespace ceinms {
    namespace Hybrid {
        namespace ObjectiveFunction {

            template<typename StaticComputationT>
            MinTorqueTrackEMGs<StaticComputationT>::MinTorqueTrackEMGs(
                StaticComputationT& staticComputation,
                double epsilon, double nEpsilon, HybridWeightings hybridParameters) :
                epsilon_(epsilon),
                nEpsilon_(nEpsilon),
                staticComputation_(staticComputation),
                hybridParameters_(hybridParameters),
                f_(DBL_MAX), fOpt_(DBL_MAX) {

                fLatest_.resize(nEpsilon_);
                for (unsigned int a = 0; a < nEpsilon_; ++a)
                    fLatest_.at(a) = DBL_MAX;

            }


            template<typename StaticComputationT>
            void MinTorqueTrackEMGs<StaticComputationT>::evalFp() {

                //1)
                //calculate first term of objective function: Least Squares Fitting

                double torqueLeastSquaresFitting = .0;
                vector<double> externalTorques, torques;
                staticComputation_.getExternalTorques(externalTorques);
                staticComputation_.getTorques(torques);
                for (unsigned d = 0; d < torques.size(); ++d)
                    torqueLeastSquaresFitting += fabs(externalTorques.at(d) - torques.at(d))*fabs(externalTorques.at(d) - torques.at(d));

                //2)
                //calculate the second term of objective function: track experimental EMGs

                double emgTracking = .0;
                vector<double> experimentalEMGs, adjustedEMGs;
                staticComputation_.getInitialValuesOfTrackedEMGs(experimentalEMGs); //emg value for the tracked muscles before the emg adjustment
                staticComputation_.getAdjustedValuesOfTrackedEMGs(adjustedEMGs);

                /*        cout << "\n\nExp vs Adj EMGS: " << experimentalEMGs.size() << "\n";
                        for (unsigned k = 0; k < experimentalEMGs.size(); ++k)
                        cout << experimentalEMGs.at(k) << "\t" << adjustedEMGs.at(k) << endl;
                        */
                for (unsigned e = 0; e < adjustedEMGs.size(); ++e)
                    emgTracking += fabs(experimentalEMGs.at(e) - adjustedEMGs.at(e));

                //3)
                //sum of all EMGs
                double emgSum = .0;
                vector<double> currentEMGs;
                //          cout << "emgs \n";
                staticComputation_.getCurrentEMGs(currentEMGs);
                for (unsigned i = 0; i < currentEMGs.size(); ++i) {
                    emgSum += currentEMGs.at(i)*currentEMGs.at(i);
                    //            cout << currentEMGs.at(i) << " ";
                }
                //      cout << endl;

                fp_ = hybridParameters_.alpha*torqueLeastSquaresFitting + hybridParameters_.beta*emgTracking + hybridParameters_.gamma*emgSum;

                /*      cout << "torquesFitting " << torqueLeastSquaresFitting << endl;
                      cout << "trackEMG_ " << emgTracking << endl;
                      cout << "sumEMG_ " << emgSum << endl; */
            }


            template<typename StaticComputationT>
            bool MinTorqueTrackEMGs<StaticComputationT>::isFacceptable() {

                if (fp_ > f_)
                    return false;
                return true;
            }


            template<typename StaticComputationT>
            void MinTorqueTrackEMGs<StaticComputationT>::updateF() {

                f_ = fp_;
            }



            template<typename StaticComputationT>
            bool MinTorqueTrackEMGs<StaticComputationT>::isFoptAcceptable() {

                if (fp_ > fOpt_)
                    return false;
                return true;
            }


            template<typename StaticComputationT>
            void MinTorqueTrackEMGs<StaticComputationT>::updateFopt() {

                fOpt_ = fp_;
            }


            template<typename StaticComputationT>
            void MinTorqueTrackEMGs<StaticComputationT>::printFs() {

                cout << "fp_: ";
                cout << fp_ << " ";
                cout << endl;
                cout << "f_: ";
                cout << f_ << " ";
                cout << endl;
                cout << "fOpt_: ";
                cout << fOpt_ << " ";
                cout << endl;
            }


            template<typename StaticComputationT>
            double MinTorqueTrackEMGs<StaticComputationT>::computeMetropolisCriteria(const double t) {

                double p = (f_ - fp_);
                //           cout << "f_ " << f_ << " fp_ " << fp_  << endl;
                return exp(p / t);
            }


            template<typename StaticComputationT>
            bool MinTorqueTrackEMGs<StaticComputationT>::terminate() {

                fLatest_.at(0) = f_;
                for (unsigned int a = 0; a < nEpsilon_; ++a) {
                    if (fabs(fLatest_.at(a) - fOpt_) > epsilon_)
                        return false;
                }
                return true;
            }



            template<typename StaticComputationT>
            void MinTorqueTrackEMGs<StaticComputationT>::updateFandFlatest() {
                for (unsigned int a = 1; a < nEpsilon_; ++a)
                    fLatest_.at(nEpsilon_ - a) = fLatest_.at(nEpsilon_ - a - 1);
                f_ = fOpt_;
            }

        }
    }
}