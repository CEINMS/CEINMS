//__________________________________________________________________________
// Author(s): Claudio Pizzolato, Monica Reggiani - March 2015
// email:  claudio.pizzolato@griffithuni.edu.au
//
// DO NOT REDISTRIBUTE WITHOUT PERMISSION
//__________________________________________________________________________
//


#ifndef Ceinms_SimulationManager_h
#define Ceinms_SimulationManager_h
#include "SetupDataStructure.h"
#include "CeinmsSetupXmlReader.h"
#include "InputDataXmlReader.h"
#include "ExecutionXmlReader.h"
#include <string>
#include <vector>
#include <map>
#include <iostream>

namespace CEINMS {
    class SimulationManager {

    public:
        SimulationManager(const std::string& setupFilename);
        bool run();

    private:
        template <typename NMSmodelT, template <typename, typename> class ModelEvaluatorT, typename LoggerT>
        bool runOpenLoop();

        template <typename T>
        bool runHybrid();

        static void sortMaFilenames(const std::map<std::string, std::string>& maMap, const std::vector< std::string >& dofNames, std::vector< std::string >& maDataFilenames);

        CeinmsSetupXmlReader ceinmsSetup_;
        InputDataXmlReader dataLocations_;
        ExecutionXmlReader executionCfg_;
        std::string subjectFile_;
        std::string executionFile_;
        std::string inputData_;
        std::string outputDirectory_;
        std::string emgGeneratorFile_;
    };
<<<<<<< HEAD
=======

    //non-member non-friend functions are the best!
    template <typename NMSmodelT>
    void setupSubject(NMSmodelT& mySubject, std::string subjectFile, double tolerance=0.0) {

        SetupDataStructure<NMSmodelT> setupData(subjectFile);
        setupData.createCurves();
        setupData.createMuscles(mySubject);
        setupData.createDoFs(mySubject);
        mySubject.setTendonTolerance(tolerance);
    }

    void sortMaFilenames(const std::map<std::string, std::string>& maMap, const std::vector< std::string >& dofNames, std::vector< std::string >& maDataFilenames)
    {
        int currentDof = 0;
        for (auto& it : dofNames)
        {
            try
            {
                maDataFilenames.push_back(maMap.at(it));
            }
            catch (std::out_of_range)
            {
                std::cerr << "Could not find moment arm file for " << it << " degree of freedom" << std::endl;
                exit(EXIT_FAILURE);
            }
        }
    }
>>>>>>> testsEquilibriumElastic
}


#include "SimulationManager.cpp"
#endif