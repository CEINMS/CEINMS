#ifndef ceinms_HybridWeightings_h
#define ceinms_HybridWeightings_h


namespace ceinms{
    struct HybridWeightings {
        HybridWeightings() :
            alpha(1),
            beta(0),
            gamma(0) {}
        double alpha, beta, gamma;
    };

}
#endif