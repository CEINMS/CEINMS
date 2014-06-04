//__________________________________________________________________________
// Author(s): Claudio Pizzolato, Monica Reggiani - October 2013
// email:  claudio.pizzolato@griffithuni.edu.au
//         monica.reggiani@gmail.com
//
// DO NOT REDISTRIBUTE WITHOUT PERMISSION
//__________________________________________________________________________
//


#ifndef ExternalTorqueFromX_h
#define ExternalTorqueFromX_h

#include <vector>
#include <string>

class ExternalTorqueFromX
{
public:
  template <typename NMSModelT>
  ExternalTorqueFromX(const NMSModelT& subject);
  virtual ~ExternalTorqueFromX();
  virtual void operator()() {};
  void updateExternalTorque(const std::vector<double>& currentExternalTorqueData, double currentTime, unsigned int whichDof); 
protected:
  std::vector< std::string > dofNames_;
};


template <typename NMSModelT>
ExternalTorqueFromX::ExternalTorqueFromX(const NMSModelT& subject)
{
  subject.getDoFNames(dofNames_);  
}

#endif
