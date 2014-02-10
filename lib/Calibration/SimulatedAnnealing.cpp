//__________________________________________________________________________
// Author(s): Claudio Pizzolato, Monica Reggiani - October 2013
// email:  claudio.pizzolato@griffithuni.edu.au
//         monica.reggiani@gmail.com
//
// DO NOT REDISTRIBUTE WITHOUT PERMISSION
//__________________________________________________________________________
//



#include "SimulatedAnnealing.h"
#include "TorquesComputation.h"

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


//#define LOG_SIMULATED_ANNEALING


template <typename ParametersT, typename ObjectiveFunctionT,typename TorquesComputationT>
SimulatedAnnealing<ParametersT, ObjectiveFunctionT, TorquesComputationT>
::SimulatedAnnealing(ParametersT& parametersPolicy, TorquesComputationT& torquesComputation, SimulatedAnnealingParameters simanParameters)
:parameters_(parametersPolicy),
objectiveFunction_(torquesComputation,  simanParameters.epsilon, simanParameters.noEpsilon) {

    noParameters_ = parameters_.getNoParameters();
	x_.resize(noParameters_);
	parameters_.getStartingVectorParameters(x_);
    parameters_.getUpperLowerBounds(upperBounds_, lowerBounds_);
	
    xOpt_.resize(noParameters_);
    v_.resize(noParameters_);
    for (int i = 0; i < noParameters_; ++i)
        v_.at(i) = (upperBounds_.at(i)-lowerBounds_.at(i))/2;    
    
	xp_.resize(noParameters_);
    noAccepted_.resize(noParameters_);
	
    nt_        = simanParameters.NT;
    ns_        = simanParameters.NS;
    rt_        = simanParameters.rt; 
    t_         = simanParameters.T;
    maxNoEval_ = simanParameters.maxNoEval;  
    srand(time(NULL));
   //srand(1); 
}


template <typename ParametersT, typename ObjectiveFunctionT,typename TorquesComputationT>
void SimulatedAnnealing<ParametersT, ObjectiveFunctionT, TorquesComputationT>
::checkBounds(int k) {
    
    if((xp_.at(k) < lowerBounds_.at(k)) || 
       (xp_.at(k) > upperBounds_.at(k)))        
        xp_.at(k) = lowerBounds_.at(k) + ((upperBounds_.at(k) - lowerBounds_.at(k)) * rand()/RAND_MAX);
}


template <typename ParametersT, typename ObjectiveFunctionT,typename TorquesComputationT>
void SimulatedAnnealing<ParametersT, ObjectiveFunctionT, TorquesComputationT>
::optimize() {

    int noEval = 0;
    for (int i = 0; i < noParameters_; ++i)
        noAccepted_.at(i) = 0;
  // DO UNTIL convergence or maxNoEvaluation
    bool terminate = false;
    while( (!terminate) && (noEval < maxNoEval_) ) {
    
        // DO Nt times
        for (int i = 0; i < nt_; ++i) {

            // DO Ns times
            for (int j = 0;  j < ns_; ++j) {

            // DO i = 1, n
            for (int k = 0; k < noParameters_; ++k) {
      
#ifdef LOG_SIMULATED_ANNEALING
                cout << "\ni: " << i << "/" << nt_ 
                    << " j: " << j << "/" << ns_
                    << " k: " << k << "/" << noParameters_ << endl;
#endif
          //:TODO: check with massimo
          // loro mettono qui (if NEVALS>=MAXEVAL)
          //  return....
          // io penso abbia piu` senso metterlo fuori
          // (condizione while)
      
#ifdef LOG_SIMULATED_ANNEALING
                cout << "x_ : ";
                for (unsigned int it = 0; it < x_.size(); ++it)
                    cout << x_.at(it) << " ";
                cout << endl;
#endif
          // x'_i = x_i + r v_i
                xp_ = x_;

                // :TODO: perche' mettono (2*rand()-1.0) invece di rand()?
                double factorForV = (2.*rand()/static_cast<double>(RAND_MAX)-1.);
                xp_.at(k) = x_.at(k) + v_.at(k) * factorForV ;    
                checkBounds(k);

#ifdef LOG_SIMULATED_ANNEALING
                cout << "xp_ : ";
                for (unsigned int it = 0; it < xp_.size(); ++it)
                    cout << xp_.at(it) << " ";
                cout << endl;
#endif
          
                parameters_.setVectorParameters(xp_);
          
                // objectiveFunction_ ha al suo interno un riferimento
                // a TorqueComputation_ che fa girare il codice del modello
                // per avere come output torque e penalty
                objectiveFunction_.evalFp();
                ++noEval;
          
#ifdef LOG_SIMULATED_ANNEALING
                cout << "eval no. " << noEval << endl; 
                objectiveFunction_.printFs();
#endif
          
            // if f' < f then
                if(objectiveFunction_.isFacceptable()) {
                    // X = X'
                    x_ = xp_;
                    // f = f'
                    objectiveFunction_.updateF();  // f_ = fp_;
                    // update statistics
                    noAccepted_.at(k)++;
            
#ifdef LOG_SIMULATED_ANNEALING
            cout << "F is ACCEPTABLE\n";
#endif
        
                    // if f' < fopt then
                    //if (fp_ < fOpt_) {
                    if(objectiveFunction_.isFoptAcceptable()) {
                        // Xopt = X'
                        xOpt_ = xp_;
                        // fOpt = f'
                        objectiveFunction_.updateFopt(); // fOpt_ = fp_;
                
#ifdef LOG_SIMULATED_ANNEALING
                        cout << "Fopt is ACCEPTABLE\n";
#endif
                    } 
                } 
                else { // IF f' > f THEN
                    double p = objectiveFunction_.computeMetropolisCriteria(t_);
                    double randomValue = rand()/static_cast<double>(RAND_MAX);

                    if(randomValue < p) {
#ifdef LOG_SIMULATED_ANNEALING
                        cout << "through Metropolis " << randomValue << "<" << p;
                        cout << " is acceptable\n";
#endif
                        x_ = xp_;             // X = X'
                        objectiveFunction_.updateF(); // f_ = fp_;
                        noAccepted_.at(k)++;
                    }
                    else {
#ifdef LOG_SIMULATED_ANNEALING
                        cout << "through Metropolis NOT ACCEPTABLE\n";
#endif
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
                v_.at(h) = v_.at(h) * ( 1 + c.at(h) * (ratio - 0.6) / 0.4 );
            else if (ratio < 0.4)
                v_.at(h) = v_.at(h) / (1 + c.at(h) * ((0.4 - ratio)/ 0.4));
            if (v_.at(h) > (upperBounds_.at(h) - lowerBounds_.at(h)))
                v_.at(h) = upperBounds_.at(h) - lowerBounds_.at(h);
        }

        for (int h = 0; h < noParameters_; ++h)
            noAccepted_.at(h) = 0.;	
   
    } // i
    
    // terminate?
    terminate = objectiveFunction_.terminate();

    // reduce temperature
    t_ = t_ * rt_;
#ifdef LOG_SIMULATED_ANNEALING
    cout << "Temperature reduced at: " << t_ << endl;
#endif
    objectiveFunction_.updateFandFlatest();
    // restart
    x_ = xOpt_;

    }  /* end while */
    parameters_.setVectorParameters(xOpt_);
    objectiveFunction_.evalFp();
    cout << "total evaluations:" << noEval << endl;
}


template <typename ParametersT, typename ObjectiveFunctionT,typename TorquesComputationT>
std::ostream& operator<< (std::ostream& output, 
                          const SimulatedAnnealing<ParametersT, ObjectiveFunctionT, TorquesComputationT>& sa)
{
    output << "NT:        " << sa.nt_ << endl;
    output << "NS:        " << sa.ns_ << endl;
    output << "RT:        " << sa.rt_ << endl;
    output << "T:         " << sa.t_  << endl;
    output << "MaxNoEval: " << sa.maxNoEval_ << endl;
    
    output << "X:  ";  
    for (unsigned int i = 0; i < sa.x_.size(); ++i)
        output << sa.x_.at(i) << " ";
    output << endl;
    
    output << "Upper Bounds:  ";
    for (unsigned int i = 0; i < sa.upperBounds_.size(); ++i)
        output << sa.upperBounds_.at(i) << " ";
    output << endl;  
    
    output << "Lower Bounds: ";
    for (unsigned int i = 0; i < sa.lowerBounds_.size(); ++i)
        output << sa.lowerBounds_.at(i) << " ";
    output << endl;
    
    return output;
}

