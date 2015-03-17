#ifndef Ceinms_SimulationManager_h
#define Ceinms_SimulationManager_h
#include "CeinmsSetupXmlReader.h"
#include "InputDataXmlReader.h"
#include "ExecutionXmlReader.h"
#include <string>

namespace CEINMS {
    class SimulationManager {

    public:
        SimulationManager(const std::string& setupFilename);
        bool run();

    private:
        template <typename NMSmodelT, template <typename, typename> typename ModelEvaluatorT, typename LoggerT>
        bool runOpenLoop();

        template <typename T>
        bool runHybrid();


        CeinmsSetupXmlReader ceinmsSetup_;
        InputDataXmlReader dataLocations_;
        ExecutionXmlReader executionCfg_;
        std::string subjectFile_;
        std::string executionFile_;
        std::string inputData_;
        std::string outputDirectory_;
        std::string emgGeneratorFile_;



    };

    //non-member non-friend functions are the best!
    template <typename NMSmodelT>
    void setupSubject(NMSmodelT& mySubject, string subjectFile) {

        SetupDataStructure<NMSmodelT> setupData(subjectFile);
        setupData.createCurves();
        setupData.createMuscles(mySubject);
        setupData.createDoFs(mySubject);

    }
}


#include "SimulationManager.cpp"
#endif