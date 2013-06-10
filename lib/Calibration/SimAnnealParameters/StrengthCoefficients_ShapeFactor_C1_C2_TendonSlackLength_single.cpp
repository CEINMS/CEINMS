// This is part of
// NeuroMuscoloSkeletal Model Software (NMS)
// Copyright (C) 2010 David Lloyd Massimo Sartori Monica Reggiani
//
// ?? Licenza ??
//
// The authors may be contacted via:
// email: massimo.sartori@gmail.com monica.reggiani@gmail.com


#include <iostream>
using std::cout;
using std::endl;

#include <string>
using std::string;

#include <vector>
using std::vector;

#include "NMSmodel.h"

// A, C1 and C2 are singulary set for each muscle

template <typename NMSmodelT>
StrengthCoefficients_ShapeFactor_C1_C2_TendonSlackLength_single<NMSmodelT>
::StrengthCoefficients_ShapeFactor_C1_C2_TendonSlackLength_single(NMSmodelT& subject, 
                                                                  vector<string>& dofToCalibrate)
:subject_(subject) {

  
  subject_.getMusclesIndexFromDofs(musclesIndexToCalibrate_, dofToCalibrate);

//  cout << "musclesIndexToCalibrate_\n";
//  for (int i = 0; i < musclesIndexToCalibrate_.size(); ++i)
//    cout << musclesIndexToCalibrate_.at(i) << "  ";
//  cout << endl;
  
  // Now group the muscles based on their strengthCoefficients
  subject_.getGroupMusclesBasedOnStrengthCoefficientsFilteredByMusclesIndexList(strengthCoefficientValues_, muscleGroups_, musclesIndexToCalibrate_);
  //subject_.getGroupMusclesBasedOnStrengthCoefficients(strengthCoefficientValues_, muscleGroups_);
  noParameters_ = 3*musclesIndexToCalibrate_.size() + strengthCoefficientValues_.size() + musclesIndexToCalibrate_.size();
}


template <typename NMSmodelT>
void StrengthCoefficients_ShapeFactor_C1_C2_TendonSlackLength_single<NMSmodelT>
::getStartingVectorParameters(vector<double>& x) {
        
  x.resize(noParameters_);
  int indexCounter;
  // at the beginning the strength coefficients
  for ( int i = 0; i < strengthCoefficientValues_.size(); ++i )
    x.at(i) = strengthCoefficientValues_.at(i);
  indexCounter = strengthCoefficientValues_.size();
  // Then we have the shapeFactor
  vector<double> shapeFactors;
  subject_.getShapeFactors(shapeFactors);
  for (int i = 0; i < musclesIndexToCalibrate_.size(); ++i)
    x.at(indexCounter+i) = shapeFactors.at(musclesIndexToCalibrate_.at(i)); 
  indexCounter += musclesIndexToCalibrate_.size();

    // the C1
  vector<double> c1Coefficients;
  subject_.getC1Coefficients(c1Coefficients);
  for (int i = 0; i < musclesIndexToCalibrate_.size(); ++i)
    x.at(indexCounter+i) = c1Coefficients.at(musclesIndexToCalibrate_.at(i)); 
  indexCounter += musclesIndexToCalibrate_.size();

    // and the C2

  vector<double> c2Coefficients;
  subject_.getC2Coefficients(c2Coefficients);
  for (int i = 0; i < musclesIndexToCalibrate_.size(); ++i)
    x.at(indexCounter+i) = c2Coefficients.at(musclesIndexToCalibrate_.at(i)); 
  indexCounter += musclesIndexToCalibrate_.size();
  
  // then, we ask the list of tendonSlackLengths
  vector<double> tendonSlackLengths;
  subject_.getTendonSlackLengths(tendonSlackLengths);
  for (int i = 0; i < musclesIndexToCalibrate_.size(); ++i)
    x.at(indexCounter+i) = tendonSlackLengths.at(musclesIndexToCalibrate_.at(i)); 
}


template <typename NMSmodelT>
void StrengthCoefficients_ShapeFactor_C1_C2_TendonSlackLength_single<NMSmodelT>
::setUpperLowerBounds(vector<double>& upperBounds, vector<double>& lowerBounds) {
       
  upperBounds.resize(noParameters_); 
  lowerBounds.resize(noParameters_);

  // at the beginning the strength coefficients
  for ( int i = 0; i < strengthCoefficientValues_.size(); ++i ) {
    upperBounds.at(i) = 2.5;
    lowerBounds.at(i) = 0.5;
    
  }  
  int indexCounter = strengthCoefficientValues_.size();
  // Then we have the shapeFactor
  for (int i = 0; i < musclesIndexToCalibrate_.size(); ++i)
  {
      upperBounds.at(indexCounter+i) = -0.001; // shape factor A 
      lowerBounds.at(indexCounter+i) = -2.999;	
  }
  indexCounter += musclesIndexToCalibrate_.size();

  // the C1

  for (int i = 0; i < musclesIndexToCalibrate_.size(); ++i)
  {
  upperBounds.at(indexCounter+i) = -0.0050;
  lowerBounds.at(indexCounter+i) = -0.9950;
  }
  indexCounter += musclesIndexToCalibrate_.size();
  
// and the C2
   for (int i = 0; i < musclesIndexToCalibrate_.size(); ++i)
  {
  upperBounds.at(indexCounter+i) = -0.0050;
  lowerBounds.at(indexCounter+i) = -0.9950;
  }
  indexCounter += musclesIndexToCalibrate_.size();
  
  // then, tendonSlackLengths
  for (int i = 0; i < musclesIndexToCalibrate_.size(); ++i) {
  // :TODO: c'era
  // (RESTING_TENDON_LENGTH) + ((RESTING_TENDON_LENGTH)*0.15); 
  // verificare che il restingTendonLength, sia il tendonSlackLength
  // then, we ask the list of tendonSlackLengths
  vector<double> tendonSlackLengths;
  subject_.getTendonSlackLengths(tendonSlackLengths);
    upperBounds.at(indexCounter+i) =  tendonSlackLengths.at(musclesIndexToCalibrate_.at(i)) + (tendonSlackLengths.at(musclesIndexToCalibrate_.at(i))*0.15); 
    lowerBounds.at(indexCounter+i) =  tendonSlackLengths.at(musclesIndexToCalibrate_.at(i)) - (tendonSlackLengths.at(musclesIndexToCalibrate_.at(i))*0.15);
  }
   	
}
 
 
template <typename NMSmodelT>
void StrengthCoefficients_ShapeFactor_C1_C2_TendonSlackLength_single<NMSmodelT>
::setVectorParameters(const vector<double>& x) {
	
		// first we set the strength coefficients
	vector<double> currentStrengthCoefficientValues;
	int indexCounter = strengthCoefficientValues_.size();
	currentStrengthCoefficientValues.resize(strengthCoefficientValues_.size());
	
	for (int i = 0; i <  strengthCoefficientValues_.size(); ++i)
		currentStrengthCoefficientValues.at(i) = x.at(i);
	
	subject_.setStrengthCoefficientsBasedOnGroups(currentStrengthCoefficientValues, muscleGroups_);
  
		// Then we have the shapeFactor
  vector<double> currentShapeFactors;
  subject_.getShapeFactors(currentShapeFactors);
  for (int i = 0; i < musclesIndexToCalibrate_.size(); ++i)
    currentShapeFactors.at(musclesIndexToCalibrate_.at(i)) = x.at(indexCounter+i);
	subject_.setShapeFactors(currentShapeFactors);	
  indexCounter += musclesIndexToCalibrate_.size();

//cout << "bp 2\n";
  	// the C1
  vector<double> currentC1Coefficients;
  subject_.getC1Coefficients(currentC1Coefficients);
  for (int i = 0; i < musclesIndexToCalibrate_.size(); ++i)
    currentC1Coefficients.at(musclesIndexToCalibrate_.at(i)) = x.at(indexCounter+i);
	subject_.setC1Coefficients(currentC1Coefficients);	
  indexCounter += musclesIndexToCalibrate_.size();

//cout << "bp 3\n";
		// and the C2
  vector<double> currentC2Coefficients;
  subject_.getC2Coefficients(currentC2Coefficients);
  for (int i = 0; i < musclesIndexToCalibrate_.size(); ++i)
    currentC2Coefficients.at(musclesIndexToCalibrate_.at(i)) = x.at(indexCounter+i);
	subject_.setC2Coefficients(currentC2Coefficients);	
  indexCounter += musclesIndexToCalibrate_.size();
	
//cout << "bp 4\n";
	  // then we set the list of tendonSlackLengths
	vector<double> currentTendonSlackLengths;
  subject_.getTendonSlackLengths(currentTendonSlackLengths);
	for ( int i = 0; i < musclesIndexToCalibrate_.size(); ++i ) 
	  currentTendonSlackLengths.at(musclesIndexToCalibrate_.at(i)) = x.at(indexCounter+i);
	subject_.setTendonSlackLengths(currentTendonSlackLengths);

}


template <typename NMSmodelT>
std::ostream& operator<< (std::ostream& output, const StrengthCoefficients_ShapeFactor_C1_C2_TendonSlackLength_single<NMSmodelT>& p) {
    
    output << "The number of muscle groups is: " << p.strengthCoefficientValues_.size() << endl;
    for ( unsigned int i = 0; i <  p.strengthCoefficientValues_.size(); ++i ) {
        output << "Value: "  << p.strengthCoefficientValues_.at(i) << endl;
        output << "Muscles: ";
        for ( unsigned int j = 0; j < p.muscleGroups_.at(i).size(); ++j )
            output << p.muscleGroups_.at(i).at(j) << " ";
        output << endl;   
    }
 return output;
}
