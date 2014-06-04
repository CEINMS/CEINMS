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


class LmtMaFromX
{
  public:    
    template <typename NMSModelT>
    LmtMaFromX(const NMSModelT& subject);
    virtual ~LmtMaFromX();
    virtual void operator()() {};
    void updateLmt(const std::vector<double>& currentLmtData, double currentTime);
    void updateMomentArms(const std::vector<double>& currentMomentArmsData, double currentTime, unsigned int whichDof); 
    
  protected:
    std::vector< std::string > musclesNames_;
    std::vector< std::string > dofNames_;
    std::vector< std::vector< std::string > > muscleNamesOnDofs_;
};


template <typename NMSModelT>
LmtMaFromX::LmtMaFromX(const NMSModelT& subject)
{ 
  subject.getMuscleNames(musclesNames_); 
  subject.getDoFNames(dofNames_);
  subject.getMuscleNamesOnDofs(muscleNamesOnDofs_);
}

#endif
