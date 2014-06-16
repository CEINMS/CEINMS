//__________________________________________________________________________
// Author(s): Claudio Pizzolato - October 2013
// email:  claudio.pizzolato@griffithuni.edu.au
//
// DO NOT REDISTRIBUTE WITHOUT PERMISSION
//__________________________________________________________________________
//


#ifndef NMSmodelPolicyTemplates_h
#define NMSmodelPolicyTemplates_h

#include <iostream>
#include "ExponentialActivation.h"
#include "PiecewiseActivation.h"
#include "StiffTendon.h"
#include "ElasticTendon.h"
#include "ElasticTendon_BiSec.h"
#include "Curve.h"

template class NMSmodel<ExponentialActivation, StiffTendon, CurveMode::Offline>;
template
std::ostream& operator<< (std::ostream& output, const NMSmodel<ExponentialActivation, StiffTendon, CurveMode::Offline>& b);

template class NMSmodel<ExponentialActivation, StiffTendon, CurveMode::Online>;
template
std::ostream& operator<< (std::ostream& output, const NMSmodel<ExponentialActivation, StiffTendon, CurveMode::Online>& b);

template class NMSmodel<PiecewiseActivation, StiffTendon, CurveMode::Offline>;
template
std::ostream& operator<< (std::ostream& output, const NMSmodel<PiecewiseActivation, StiffTendon, CurveMode::Offline>& b);

template class NMSmodel<PiecewiseActivation, StiffTendon, CurveMode::Online>;
template
std::ostream& operator<< (std::ostream& output, const NMSmodel<PiecewiseActivation, StiffTendon, CurveMode::Online>& b);



// template class NMSmodel<ExponentialActivation, ElasticTendon<CurveMode::Offline>, CurveMode::Offline>;
// template
// std::ostream& operator<< (std::ostream& output, const NMSmodel<ExponentialActivation, ElasticTendon<CurveMode::Offline>, CurveMode::Offline>& b);
// 
// template class NMSmodel<ExponentialActivation, ElasticTendon<CurveMode::Online>, CurveMode::Online>;
// template
// std::ostream& operator<< (std::ostream& output, const NMSmodel<ExponentialActivation, ElasticTendon<CurveMode::Offline>, CurveMode::Online>& b);
// 
// template class NMSmodel<PiecewiseActivation, ElasticTendon<CurveMode::Offline>, CurveMode::Offline>;
// template
// std::ostream& operator<< (std::ostream& output, const NMSmodel<PiecewiseActivation, ElasticTendon<CurveMode::Offline>, CurveMode::Offline>& b);
// 
// template class NMSmodel<PiecewiseActivation, ElasticTendon<CurveMode::Offline>, CurveMode::Online>;
// template
// std::ostream& operator<< (std::ostream& output, const NMSmodel<PiecewiseActivation, ElasticTendon<CurveMode::Offline>, CurveMode::Online>& b);
// 



template class NMSmodel<ExponentialActivation, ElasticTendon_BiSec, CurveMode::Offline>;
template
std::ostream& operator<< (std::ostream& output, const NMSmodel<ExponentialActivation, ElasticTendon_BiSec, CurveMode::Offline>& b);

template class NMSmodel<ExponentialActivation, ElasticTendon_BiSec, CurveMode::Online>;
template
std::ostream& operator<< (std::ostream& output, const NMSmodel<ExponentialActivation, ElasticTendon_BiSec, CurveMode::Online>& b);

template class NMSmodel<PiecewiseActivation, ElasticTendon_BiSec, CurveMode::Offline>;
template
std::ostream& operator<< (std::ostream& output, const NMSmodel<PiecewiseActivation, ElasticTendon_BiSec, CurveMode::Offline>& b);

template class NMSmodel<PiecewiseActivation, ElasticTendon_BiSec, CurveMode::Online>;
template
std::ostream& operator<< (std::ostream& output, const NMSmodel<PiecewiseActivation, ElasticTendon_BiSec, CurveMode::Online>& b);


#endif