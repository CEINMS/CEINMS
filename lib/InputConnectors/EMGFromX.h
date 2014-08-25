//__________________________________________________________________________
// Author(s): Claudio Pizzolato, Monica Reggiani - June 2014
// email:  claudio.pizzolato@griffithuni.edu.au
//         monica.reggiani@gmail.com
//
// DO NOT REDISTRIBUTE WITHOUT PERMISSION
//__________________________________________________________________________
//
 
#ifndef EMGFromX_h
#define EMGFromX_h

#include <string>
#include <vector>
#include "InputQueues.h"
class EMGFromX 
{
public:
  template <typename NMSModelT>
  EMGFromX(CEINMS::InputConnectors& inputConnectors, const NMSModelT& subject);
  virtual ~EMGFromX();
  virtual void operator()() {};
  void updateEmg(const std::vector<double>& currentEmgData, double currentTime);  
protected:
  std::vector< std::string > musclesNames_;
  CEINMS::InputConnectors& inputConnectors_;
};


template <typename NMSModelT>
EMGFromX::EMGFromX(CEINMS::InputConnectors& inputConnectors, const NMSModelT& subject) : inputConnectors_(inputConnectors)
{ 
  subject.getMuscleNames(musclesNames_);
}

#endif
