// This is part of
// NeuroMuscoloSkeletal Model Software (NMS)
// Copyright (C) 2010 David Lloyd Massimo Sartori Monica Reggiani
//
// ?? Licenza ??
//
// The authors may be contacted via:
// email: massimo.sartori@gmail.com monica.reggiani@gmail.com

#ifndef StrengthCoefficients_ShapeFactor_C1_C2_TendonSlackLength_single_h
#define StrengthCoefficients_ShapeFactor_C1_C2_TendonSlackLength_single_h

#include <iostream>
#include <string>
#include <vector>

template<typename NMSmodelT>
class StrengthCoefficients_ShapeFactor_C1_C2_TendonSlackLength_single;

template<typename NMSmodelT>
std::ostream& operator<< (std::ostream& output, 
                const StrengthCoefficients_ShapeFactor_C1_C2_TendonSlackLength_single<NMSmodelT>& p);


template <typename NMSmodelT>
class StrengthCoefficients_ShapeFactor_C1_C2_TendonSlackLength_single {
public:
    StrengthCoefficients_ShapeFactor_C1_C2_TendonSlackLength_single(NMSmodelT& subject, std::vector< std::string >& dofToCalibrate);
    // the number of parameters is strength coeffs (3) + tendon slack lengths (= no Muscles) + SlackLength + C1 + C2
    int getNoParameters() { return noParameters_; }
    void getStartingVectorParameters(std::vector<double>& x);
    void setVectorParameters(const std::vector<double>& x);
    void setUpperLowerBounds(std::vector<double>& upperBounds, std::vector<double>& lowerBounds);
    friend std::ostream& operator<< <> (std::ostream& output, 
                  const StrengthCoefficients_ShapeFactor_C1_C2_TendonSlackLength_single& p);
private:
    NMSmodelT& subject_;
    std::vector<double> strengthCoefficientValues_;
    std::vector<unsigned int> musclesIndexToCalibrate_;
    std::vector< std::vector<int> > muscleGroups_; // each index is a vector of id for the muscles
    int noParameters_;
};

#include "StrengthCoefficients_ShapeFactor_C1_C2_TendonSlackLength_single.cpp"

#endif
