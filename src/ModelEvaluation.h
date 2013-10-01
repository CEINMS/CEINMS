//__________________________________________________________________________
// Author(s): Claudio Pizzolato, Monica Reggiani - September 2013
// email:  claudio.pizzolato@griffithuni.edu.au
//
// DO NOT REDISTRIBUTE WITHOUT PERMISSION
//__________________________________________________________________________
//


#ifndef ModelEvaluation_h
#define ModelEvaluation_h

#include <vector>
#include <string>
#include "NMSmodel.h"
#include "SetupDataStructure.h"

#include "ExponentialActivation.h"
#include "StiffTendon.h"
#include "ElasticTendon.h"
#include "ElasticTendon_BiSec.h"

typedef StiffTendon MyTendonType;


class ModelEvaluation
{
  public:
    typedef NMSmodel<ExponentialActivation, MyTendonType> MyNMSmodel;
    typedef SetupDataStructure<ExponentialActivation, MyTendonType> MySetupDataStructure;
    ModelEvaluation(const std::string& configurationFile);
    ~ModelEvaluation();
    void operator()();
  private:
    void popEmgFront(std::vector<double>& emgFromQueue);
    void popLmtFront(std::vector<double>& lmtFromQueue);
    void popMomentArmsFront(std::vector<double>& momentArmsFromQueue, unsigned int whichDof);
    void popExternalTorqueFront(std::vector<double>& externalTorqueFromQueue, unsigned int whichDof);    
    void getMusclesNames();
    void setDofNames();
    void getDofNamesAssociatedToExternalTorque();
    void initOfflineCurve(MyNMSmodel& mySubject);
    std::string configurationFile_;
    std::vector< std::string > musclesNames_;
    std::vector< std::vector < std::string > > musclesNamesOnDof_;
    std::vector< std::string > dofNames_;
    std::vector< std::string > dofNamesWithExtTorque_;
    unsigned int noDof_;
};


#endif
