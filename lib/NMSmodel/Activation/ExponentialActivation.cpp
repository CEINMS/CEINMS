//__________________________________________________________________________
// Author(s): Claudio Pizzolato, Monica Reggiani - October 2013
// email:  claudio.pizzolato@griffithuni.edu.au
//         monica.reggiani@gmail.com
//
// DO NOT REDISTRIBUTE WITHOUT PERMISSION
//__________________________________________________________________________
//


#include "ExponentialActivation.h"
#include <math.h>

ExponentialActivation::ExponentialActivation():
beta1_(0.0), beta2_(0.0), alpha_(0.0), shapeFactor_(0.0),   
emg_(0.0), noOfEmg_(0), pastEmgT1_(0.0), pastEmgT2_(0.0), neuralActivation_(0.0),   
neuralActivationT1_(0.0), neuralActivationT2_(0.0), activation_(0.0), expShapeFactor_(0.0)

{
    
}

void ExponentialActivation::setFilterParameters (double c1, double c2) {

    beta1_ = c1 + c2;
    beta2_ = c1 * c2;
    alpha_ = 1 + beta1_ + beta2_;
}


void ExponentialActivation::setShapeFactor (double shapeFactor) {

    shapeFactor_ = shapeFactor;
    expShapeFactor_ = exp(shapeFactor_);
}

void ExponentialActivation::setEmg (double emg) {
    
    emg_ = emg;
}


void ExponentialActivation::updateActivation() {

    if ( noOfEmg_ == 2 ) {
        neuralActivationT1_ = (pastEmgT2_ + pastEmgT1_)/2;
        neuralActivationT2_ = neuralActivationT1_;
    } 
    
    neuralActivation_ = (alpha_*emg_) - (beta1_*neuralActivationT1_) - (beta2_*neuralActivationT2_);
    activation_ = ( exp(shapeFactor_*neuralActivation_) - 1 ) / (expShapeFactor_ - 1);
}


void ExponentialActivation::pushState() {

    neuralActivationT2_ = neuralActivationT1_;
    neuralActivationT1_ = neuralActivation_;
    pastEmgT2_ = pastEmgT1_;
    pastEmgT1_ = emg_;
    noOfEmg_++;
}


void ExponentialActivation::resetState() {

    emg_ = .0;
    activation_ = .0;
    neuralActivation_ = .0;
    neuralActivationT1_ = .0;
    neuralActivationT2_ = .0;
    pastEmgT1_ = .0;
    pastEmgT2_ = .0;
    noOfEmg_ = 0;
}


ExponentialActivation::~ExponentialActivation() { } 

