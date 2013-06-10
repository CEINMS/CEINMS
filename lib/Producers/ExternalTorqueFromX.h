#ifndef ExternalTorqueFromX_h
#define ExternalTorqueFromX_h

#include <vector>
#include <string>

class ExternalTorqueFromX
{
  public:
    virtual ~ExternalTorqueFromX();
    virtual void operator()() {};
    void pushExternalTorqueBack(const std::vector<double>& newExternalTorqueToPush, unsigned int whichDof);
    void updateExternalTorque(const std::vector<double>& currentExternalTorqueData, double currentTime, unsigned int whichDof); 
    void getDofNames(std::vector< std::string >& dofNamesFromModel);
    void setExternalTorqueDofNames(std::vector<std::string> dofNamesWithExternalTorqueFromInput);
   
};

#endif
