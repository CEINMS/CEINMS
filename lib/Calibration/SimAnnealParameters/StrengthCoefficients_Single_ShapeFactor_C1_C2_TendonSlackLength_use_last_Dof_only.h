//__________________________________________________________________________
// Author(s): Claudio Pizzolato, Monica Reggiani - September 2013
// email:  claudio.pizzolato@griffithuni.edu.au
//         monica.reggiani@gmail.com
//
// DO NOT REDISTRIBUTE WITHOUT PERMISSION
//__________________________________________________________________________
//


#ifndef StrengthCoefficients_Single_ShapeFactor_C1_C2_TendonSlackLength_use_last_Dof_only_h
#define StrengthCoefficients_Single_ShapeFactor_C1_C2_TendonSlackLength_use_last_Dof_only_h

#include <iostream>
#include <string>
#include <vector>
#include "NMSmodel.h"
#include "Curve.h"

template<typename Activation, typename Tendon>
class StrengthCoefficients_Single_ShapeFactor_C1_C2_TendonSlackLength_use_last_Dof_only;

template<typename Activation, typename Tendon>
std::ostream& operator<< (std::ostream& output, 
                const StrengthCoefficients_Single_ShapeFactor_C1_C2_TendonSlackLength_use_last_Dof_only<Activation, Tendon >& p);


template<typename Activation, typename Tendon>
class StrengthCoefficients_Single_ShapeFactor_C1_C2_TendonSlackLength_use_last_Dof_only {
public:
    typedef NMSmodel<Activation, Tendon, CurveMode::Offline> NMSmodelT;
	StrengthCoefficients_Single_ShapeFactor_C1_C2_TendonSlackLength_use_last_Dof_only(NMSmodelT& subject, std::vector< std::string >& dofToCalibrate);
	// the number of parameters is strength coeffs (3) + tendon slack lengths (= no Muscles) + SlackLength + C1 + C2
	int getNoParameters() { return noParameters_; }
	void getStartingVectorParameters(std::vector<double>& x);
	void setVectorParameters(const std::vector<double>& x);
	void setUpperLowerBounds(std::vector<double>& upperBounds, std::vector<double>& lowerBounds);
  friend std::ostream& operator<< <>(std::ostream& output, 
                  const StrengthCoefficients_Single_ShapeFactor_C1_C2_TendonSlackLength_use_last_Dof_only& p);
private:
  //  StrengthCoefficients_ShapeFactor_C1_C2_TendonSlackLength() {};
	NMSmodelT& subject_;
	std::vector<double> strengthCoefficientValues_;
  std::vector<unsigned int> musclesIndexToCalibrate_;
	std::vector< std::vector<int> > muscleGroups_; // each index is a vector of id for the muscles
 // std::vector< std::vector<int> > filteredMuscleGroups_;
	int noParameters_;
};

#include "StrengthCoefficients_Single_ShapeFactor_C1_C2_TendonSlackLength_use_last_Dof_only.cpp"

#endif
