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
    virtual ~LmtMaFromX();
    virtual void operator()() {};
    void pushLmtBack(const std::vector<double>& newLmtToPush);
    void pushMomentArmsBack(const std::vector<double>& newMomentArmsToPush, unsigned int whichDof);
    void updateLmt(const std::vector<double>& currentLmtData, double currentTime);
    void updateMomentArms(const std::vector<double>& currentMomentArmsData, double currentTime, unsigned int whichDof); 
    void getDofNames(std::vector< std::string >& dofNamesFromModel);
	void setNoDof(unsigned nDof);
	void setLmtMusclesNames(const std::vector< std::string >& lmtMusclesNames);
    void setMomentArmsMusclesNames(const std::vector< std::vector < std::string > >& musclesNamesFromMomentArmsFiles);
};

#endif
