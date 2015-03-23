//#include "BatchEvaluator.h"
#include "InputConnectors.h"
#include "OutputConnectors.h"
#include "QueuesToTrialData.h"
#include "EMGFromFile.h"
#include "LmtMaFromStorageFile.h"
#include "ExternalTorquesFromStorageFile.h"
#include "InputDataXmlReader.h"
#include <thread>
#include <vector>
using std::vector;
#include <string>
using std::string;


template<typename NMSmodelT>
TrialData readTrialData(std::string inputDataFilename, NMSmodelT& mySubject, std::string trialId, std::string emgGeneratorFile)
{
    InputDataXmlReader dataLocations(inputDataFilename);
    // CEINMS::InputConnectors* inputConnectors= new CEINMS::InputConnectors();
    //CEINMS::OutputConnectors* outputConnectors = new CEINMS::OutputConnectors();

    std::unique_ptr<InputConnectors> inputConnectors(new InputConnectors);
    std::unique_ptr<OutputConnectors> outputConnectors(new OutputConnectors);

    string emgFilename(dataLocations.getExcitationsFile());
    EMGFromFile emgProducer(*inputConnectors, mySubject, emgFilename, emgGeneratorFile);

    vector< string > dofNames;
    mySubject.getDoFNames(dofNames);
    vector< string > maFilename;
    sortMaFilenames(dataLocations.getMaFiles(), dofNames, maFilename);
    LmtMaFromStorageFile lmtMaProducer(*inputConnectors, mySubject, dataLocations.getLmtFile(), maFilename);

    string externalTorqueFilename(dataLocations.getExternalTorqueFile());
    ExternalTorquesFromStorageFile externalTorquesProducer(*inputConnectors, mySubject, externalTorqueFilename);

    QueuesToTrialData queuesToTrialData(*inputConnectors, *outputConnectors, mySubject, trialId);


    inputConnectors->doneWithSubscription.setCount(4);
    outputConnectors->doneWithExecution.setCount(1);

    // 4. start the threads
    std::thread emgProdThread(std::ref(emgProducer));
    std::thread externalTorquesProdThread(std::ref(externalTorquesProducer));
    std::thread lmtMaProdThread(std::ref(lmtMaProducer));
    std::thread queuesToTrialDataThread(std::ref(queuesToTrialData));

    emgProdThread.join();
    lmtMaProdThread.join();
    externalTorquesProdThread.join();
    queuesToTrialDataThread.join();

    if (queuesToTrialData.getTrialData().noLmtSteps_ < 1)
    {
        std::cout << "ERROR: No data was read for trial " << trialId << std::endl;
        exit(EXIT_FAILURE);
    }
    return queuesToTrialData.getTrialData();
}

int main() { 


    return 0;
}