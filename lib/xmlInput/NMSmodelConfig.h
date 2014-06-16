//__________________________________________________________________________
// Author(s): Claudio Pizzolato - October 2013
// email:  claudio.pizzolato@griffithuni.edu.au
//
// DO NOT REDISTRIBUTE WITHOUT PERMISSION
//__________________________________________________________________________
//


#ifndef NMSModelCfg_h
#define NMSModelCfg_h

namespace NMSModelCfg {
    
    const unsigned OpenLoop = 1000;
    const unsigned Hybrid   = 2000;
                 
    const unsigned ExponentialActivation = 100;
    const unsigned PiecewiseActivation   = 200;
                 
    const unsigned StiffTendon        = 10;
    const unsigned ElasticTendon      = 20;
    const unsigned ElasticTendonBiSec = 30;  
                 
    const unsigned Online  = 1;
    const unsigned Offline = 2;
                 
    const unsigned Nop = 0;
                
    
    enum RunMode{
                 
                 OpenLoopExponentialActivationStiffTendonOnline          = 1111, 
                 OpenLoopExponentialActivationStiffTendonOffline         = 1112,
                 OpenLoopExponentialActivationElasticTendonOnline        = 1121, 
                 OpenLoopExponentialActivationElasticTendonOffline       = 1122, 
                 OpenLoopExponentialActivationElasticTendonBiSecOnline   = 1131, 
                 OpenLoopExponentialActivationElasticTendonBiSecOffline  = 1132,
                 
                 OpenLoopPiecewiseActivationStiffTendonOnline            = 1211, 
                 OpenLoopPiecewiseActivationStiffTendonOffline           = 1212,
                 OpenLoopPiecewiseActivationElasticTendonOnline          = 1221, 
                 OpenLoopPiecewiseActivationElasticTendonOffline         = 1222, 
                 OpenLoopPiecewiseActivationElasticTendonBiSecOnline     = 1231, 
                 OpenLoopPiecewiseActivationElasticTendonBiSecOffline    = 1232,
                 
                 HybridExponentialActivationStiffTendonOnline            = 2111, 
                 HybridExponentialActivationStiffTendonOffline           = 2112, 
                 HybridExponentialActivationElasticTendonOnline          = 2121, 
                 HybridExponentialActivationElasticTendonOffline         = 2122, 
                 HybridExponentialActivationElasticTendonBiSecOnline     = 2131, 
                 HybridExponentialActivationElasticTendonBiSecOffline    = 2132,
                 
                 HybridPiecewiseActivationStiffTendonOnline              = 2211, 
                 HybridPiecewiseActivationStiffTendonOffline             = 2212,
                 HybridPiecewiseActivationElasticTendonOnline            = 2221, 
                 HybridPiecewiseActivationElasticTendonOffline           = 2222, 
                 HybridPiecewiseActivationElasticTendonBiSecOnline       = 2231, 
                 HybridPiecewiseActivationElasticTendonBiSecOffline      = 2232,
                 };
};

#endif