//__________________________________________________________________________
// Author(s): Claudio Pizzolato, Monica Reggiani - June 2014
// email:  claudio.pizzolato@griffithuni.edu.au
//         monica.reggiani@gmail.com
//
// DO NOT REDISTRIBUTE WITHOUT PERMISSION
//__________________________________________________________________________
//


#ifndef ExternalTorquesFromX_h
#define ExternalTorquesFromX_h

#include <vector>
#include <string>

class ExternalTorquesFromX
{
public:
  template <typename NMSModelT>
  ExternalTorquesFromX(const NMSModelT& subject);
  virtual ~ExternalTorquesFromX();
  virtual void operator()() {};
  void updateExternalTorques(const std::vector<double>& currentExternalTorquesData, double currentTime); 
protected:
  std::vector< std::string > dofNames_;
};


template <typename NMSModelT>
ExternalTorquesFromX::ExternalTorquesFromX(const NMSModelT& subject)
{
  subject.getDoFNames(dofNames_);  
}

#endif
