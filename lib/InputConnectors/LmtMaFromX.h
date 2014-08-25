//__________________________________________________________________________
// Author(s): Claudio Pizzolato, Monica Reggiani - October 2013
// email:  claudio.pizzolato@griffithuni.edu.au
//         monica.reggiani@gmail.com
//
// DO NOT REDISTRIBUTE WITHOUT PERMISSION
//__________________________________________________________________________
//


#ifndef LmtMaFromX_h
#define LmtMaFromX_h

#include <vector>
#include <string>
#include "InputQueues.h"

class LmtMaFromX
{
  public:    
    template <typename NMSModelT>
    LmtMaFromX(CEINMS::InputConnectors& inputConnectors, const NMSModelT& subject);
    virtual ~LmtMaFromX();
    virtual void operator()() {};
    void updateLmt(const std::vector<double>& currentLmtData, double currentTime);
    void updateMomentArms(const std::vector<double>& currentMomentArmsData, double currentTime, unsigned int whichDof); 
    
  protected:
    std::vector< std::string > musclesNames_;
    std::vector< std::string > dofNames_;
    std::vector< std::vector< std::string > > muscleNamesOnDofs_;
    CEINMS::InputConnectors& inputConnectors_;
};


template <typename NMSModelT>
LmtMaFromX::LmtMaFromX(CEINMS::InputConnectors& inputConnectors, const NMSModelT& subject) : inputConnectors_(inputConnectors)
{ 
  subject.getMuscleNames(musclesNames_); 
  subject.getDoFNames(dofNames_);
  subject.getMuscleNamesOnDofs(muscleNamesOnDofs_);
  /*
  std::cout << "SIZE:" << CEINMS::InputConnectors::queueMomentArms.size() << std::endl; 
  CEINMS::InputConnectors::queueMomentArms.resize(3);
  */
//   for (int i= 0; i < dofNames_.size(); ++i) {
//     CEINMS::Concurrency::Queue< std::vector<double> > newQueue;
//     CEINMS::InputConnectors::queueMomentArms.push_back(newQueue); 
//   }
}

#endif
