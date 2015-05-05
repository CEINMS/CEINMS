//__________________________________________________________________________
// Author(s): Claudio Pizzolato, Monica Reggiani - April 2015
// email:  claudio.pizzolato@griffithuni.edu.au
//
// DO NOT REDISTRIBUTE WITHOUT PERMISSION
//__________________________________________________________________________
//


#ifndef Ceinms_CalibrationManager_h
#define Ceinms_CalibrationManager_h
#include <string>
#include <list>
#include "CeinmsCalibrationSetupXmlReader.h"
#include "CalibrationXmlReader.h"
#include "InputDataXmlReader.h"
#include "TrialData.h"


namespace ceinms {

    class CalibrationManager {

    public:
        CalibrationManager(const std::string& setupFilename);
        bool run();

    private:
        template <typename NMSmodelT>
        bool runOpenLoop();
        template <typename NMSmodelT>
        bool runHybrid();
        template <typename NMSmodelT, template <typename> class OptimizerT>
        bool runMinimiseTorque(const CalibrationStep& currentCalibrationStep);

        template <typename NMSmodelT>
        static TrialData readTrialData(const std::string& inputDataFilename, 
                                       NMSmodelT& mySubject, 
                                       const std::string& trialId, 
                                       const std::string& emgGeneratorFile);
        static void sortMaFilenames(const std::map<std::string, 
                                    std::string>& maMap, 
                                    const std::vector< std::string >& dofNames, 
                                    std::vector< std::string >& maDataFilenames);

        template<typename NMSmodelT>
        static void writeXml(NMSmodelT& mySubject,
                             const std::string& uncalibratedSubjectXmlFile,
                             const string& calibrationXmlFile,
                             const string& newCalibratedSubjectXmlFile);

        CeinmsCalibrationSetupXmlReader ceinmsCalibrationSetup_;
        CalibrationXmlReader calibrationXmlReader_;
        std::string uncalibratedSubjectFile_;
        std::string calibratedSubjectFile_;  
        std::string calibrationFile_;
        std::string emgGeneratorFile_;
        std::list<std::string> calibrationTrialIDs_;

    };
}

#include "CalibrationManager.cpp"
#endif