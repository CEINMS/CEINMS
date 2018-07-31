#include "OpenSim/OpenSim.h"
#include "ceinms/concurrency/Queue.h"
#include "ceinms/concurrency/Latch.h"
#include "mcbs/Spline.h"
#include "rtb/Filter/Filter.h"
#include "rtb/plsr/plsr.h"


#include "ceinms/CeinmsSetupXmlReader.h"
#include "ceinms/NMSmodel.h"

#include "ceinms/Tendon/StiffTendon.h"
#include "ceinms/Tendon/ElasticTendon.h"
#include "ceinms/Tendon/ElasticTendon_BiSec.h"

#include "ceinms/Activation/ExponentialActivation.h"
#include "ceinms/Activation/PiecewiseActivation.h"
#include "ceinms/Curve.h"

#include "ceinms/EmgFromFile.h"
#include "ceinms/LmtMaFromStorageFile.h"
#include "ceinms/ExternalTorquesFromStorageFile.h"
#include "ceinms/GeneralisedCoordinatesFromStorageFile.h"
#include "ceinms/ExternalForcesFromStorageFile.h"

#include "ceinms/ModelEvaluationOnline.h"
#include "ceinms/ModelEvaluationOffline.h"
#include "ceinms/ModelEvaluationHybrid.h"
#include "ceinms/QueueToStorageFile.h"
#include "ceinms/QueuesToStorageFiles.h"
#include "ceinms/LoggerOnQueues.h"
#include "ceinms/InputDataXmlReader.h"

#include "ceinms/queues/MuscleForcesQueue.h"
#include "ceinms/queues/ScalarVectorQueue.h"
#include "rtosim/TimeDifference.h"
#include <numeric>
#include <vector>
#include <list>
using namespace ceinms;
using ceinms::Concurrency::Latch;
using ceinms::Concurrency::Queue;

using NMSmodelT = NMSmodel<ExponentialActivation, ElasticTendon_BiSec, CurveMode::Online>;
SimTK::Matrix parseCSV(const std::string& filename);
void printCSV(const SimTK::MatrixView m, const std::string& outputFilename);
void printCSV(const std::list<std::vector<double>> m, const std::string& outputFilename);
void sortMaFilenames(const std::map<std::string, std::string>& maMap, const std::vector< std::string >& dofNames, std::vector< std::string >& maDataFilenames);


class AchillesTendonModel {

public:
    AchillesTendonModel(
        MuscleForcesQueue& inputMuscleForcesQueue,
        ScalarVectorQueue& outputStrainQueue,
        Latch& doneWithSubscriptions,
        Latch& doneWithExecution,
        const std::vector<std::string>& allMuscleNames,
        rtb::Plsr strainPls) :
        inputMuscleForcesQueue_(inputMuscleForcesQueue),
        outputStrainQueue_(outputStrainQueue),
        doneWithSubscriptions_(doneWithSubscriptions),
        doneWithExecution_(doneWithExecution),
        allMuscleNames_(allMuscleNames),
        strainPls_(strainPls)
    {

    }
    void operator_good() {

        int nCols = strainPls_.getB().ncol();
        const vector<string> atMuscleNames = { "lat_gas_r",	"med_gas_r", "soleus_r" };
        Mapper muscleMapper(allMuscleNames_, atMuscleNames);
        SimTK::Matrix atForce(1, 1);
        inputMuscleForcesQueue_.subscribe();
        doneWithSubscriptions_.wait();

        bool runCondition(true);
        while (runCondition) {
            MuscleForcesFrame muscleForcesFrame = inputMuscleForcesQueue_.pop();
            if (EndOfData::isEod(muscleForcesFrame)) {
                runCondition = false;
            }
            if (runCondition) {
                vector<double> atMuscleForces = muscleMapper.map(muscleForcesFrame.data);
                atForce(0, 0) = std::accumulate(atMuscleForces.begin(), atMuscleForces.end(), 0.);
                SimTK::Matrix strains(strainPls_.project(atForce.getAsMatrixView()));
                ScalarVectorFrame outputStrains;
                outputStrains.time = muscleForcesFrame.time;
                outputStrains.data.resize(strains.ncol());
                for (size_t i(0); i < strains.ncol(); ++i) {
                    outputStrains.data.at(i) = strains(0, i);
                }
                outputStrainQueue_.push(outputStrains);
            }
            else {
                outputStrainQueue_.push(EndOfData::get<ScalarVectorFrame>());
            }
        }
        doneWithExecution_.wait();
    }

    void operator()() {

        int nCols = strainPls_.getB().ncol();
        std::vector<double> B, meanY;
        for (int i(0); i < nCols; ++i) {
            B.push_back(strainPls_.getB()(0, i));
        }
        for (int i(0); i < nCols; ++i) {
            meanY.push_back(strainPls_.getMeanY()(i));
        }
        double meanX = strainPls_.getMeanX()(0);

        const vector<string> atMuscleNames = { "lat_gas_r",	"med_gas_r", "soleus_r" };
        Mapper muscleMapper(allMuscleNames_, atMuscleNames);
        inputMuscleForcesQueue_.subscribe();
        doneWithSubscriptions_.wait();

        bool runCondition(true);
        while (runCondition) {
            MuscleForcesFrame muscleForcesFrame = inputMuscleForcesQueue_.pop();
            if (EndOfData::isEod(muscleForcesFrame)) {
                runCondition = false;
            }
            if (runCondition) {
                vector<double> atMuscleForces = muscleMapper.map(muscleForcesFrame.data);
                double atForce = std::accumulate(atMuscleForces.begin(), atMuscleForces.end(), 0.) -meanX;
                ScalarVectorFrame outputStrains;
                outputStrains.time = muscleForcesFrame.time;
                outputStrains.data.resize(nCols);
                for (size_t i(0); i < nCols; ++i) {
                    outputStrains.data.at(i) = (B[i] * atForce) + meanY[i];
                }
                outputStrainQueue_.push(outputStrains);
            }
            else {
                outputStrainQueue_.push(EndOfData::get<ScalarVectorFrame>());
            }
        }
        doneWithExecution_.wait();
    }


private:
    MuscleForcesQueue& inputMuscleForcesQueue_;
    ScalarVectorQueue& outputStrainQueue_;
    Latch& doneWithSubscriptions_;
    Latch& doneWithExecution_;
    std::vector<std::string> allMuscleNames_;
    rtb::Plsr strainPls_;
};




class QueueToCSV {

public:
    QueueToCSV(
        ScalarVectorQueue& inputScalarVectorQueue,
        Latch& doneWithSubscriptions,
        Latch& doneWithExecution,
        const string& outputFilename) :
        inputScalarVectorQueue_(inputScalarVectorQueue),
        doneWithSubscriptions_(doneWithSubscriptions),
        doneWithExecution_(doneWithExecution),
        outputFilename_(outputFilename)
    {}

    void operator()() {
    
        inputScalarVectorQueue_.subscribe();
        doneWithSubscriptions_.wait();

        bool runCondition(true);
        while (runCondition) {
            ScalarVectorFrame scalarVectorFrame = inputScalarVectorQueue_.pop();
            if (EndOfData::isEod(scalarVectorFrame)) {
                runCondition = false;
            }
            if (runCondition) {
                data_.push_back(scalarVectorFrame.data);
            }
        }
        doneWithExecution_.wait();
        // Now print to file
        printCSV(data_, outputFilename_);
    }



private:
    ScalarVectorQueue& inputScalarVectorQueue_;
    Latch& doneWithSubscriptions_;
    Latch& doneWithExecution_;
    const string outputFilename_;
    std::list<std::vector<double>> data_;

};

int main() {

    string subjectFile = "C:\\Users\\s2849511\\Data\\ABC11\\TestData\\20151214\\ceinms\\subjects\\calibrated\\calibrated0.xml";
    string trialFilename = "C:\\Users\\s2849511\\Data\\ABC11\\TestData\\20151214\\ceinms\\trials\\baseline_walking.xml";
    string emgGeneratorFile = "C:\\Users\\s2849511\\Data\\ABC11\\TestData\\20151214\\ceinms\\excitationGenerator\\excitationGenerator16to34-RIGHT.xml";
    string xFilename("C:\\Users\\s2849511\\Data\\ABC11\\analysis\\strainX.csv");
    string yFilename("C:\\Users\\s2849511\\Data\\ABC11\\analysis\\strainY.csv");
    string strainoutput("C:\\Users\\s2849511\\Data\\ABC11\\output\\strain.csv");
    string outputDirectory("C:\\Users\\s2849511\\Data\\ABC11\\output");

    
    
    InputDataXmlReader dataLocations(trialFilename);

    //Create queues for input data
    EmgQueue emgQueue;
    LmtQueue lmtQueue;
    ExternalTorquesQueue externalTorqueQueue;
    MultipleMomentArmsQueues momentArmsQueues;

    //Creates queues for output data
    ScalarLogQueue logQueue;
    ScalarVectorQueue strainQueue;

    //Creates latches to sync threads
    Latch doneWithSubscriptionsLatch;
    Latch doneWithExecutionLatch;
    
    NMSmodelT mySubject;
    setupSubject(mySubject, subjectFile);

    // 2. define the thread connecting with the input sources
    string emgFilename(dataLocations.getExcitationsFile());
    EmgFromFile emgProducer(emgQueue, doneWithSubscriptionsLatch, doneWithExecutionLatch, mySubject, emgFilename, emgGeneratorFile);

    vector< string > dofNames;
    mySubject.getDoFNames(dofNames);
    vector< string > maFilename;
    sortMaFilenames(dataLocations.getMaFiles(), dofNames, maFilename);
    LmtMaFromStorageFile lmtMaProducer(lmtQueue, momentArmsQueues, doneWithSubscriptionsLatch, doneWithExecutionLatch, mySubject, dataLocations.getLmtFile(), maFilename);

    string externalTorqueFilename(dataLocations.getExternalTorqueFile());
    ExternalTorquesFromStorageFile externalTorquesProducer(externalTorqueQueue, doneWithSubscriptionsLatch, doneWithExecutionLatch, mySubject, externalTorqueFilename);

    vector<string> dataToLog = { 
        "Activations",
        "FibreLengths",
        "NormFibreLengths",
        "FibreVelocities",
        "NormFibreVelocities",
        "PennationAngles",
        "MuscleForces",
        "Torques",
        "MusclesContribution" };
    // 2b. define the thread consuming the output sources
    vector<string> valuesToWrite = dataToLog;
    QueuesToStorageFiles queuesToStorageFiles(logQueue, doneWithSubscriptionsLatch, doneWithExecutionLatch, mySubject, valuesToWrite, outputDirectory);

    // 3. define the model simulator
    vector<string> valuesToLog = dataToLog;
    ModelEvaluationOnline<NMSmodelT, LoggerOnQueues>  simulator(
        emgQueue,
        lmtQueue,
        externalTorqueQueue,
        momentArmsQueues,
        logQueue,
        doneWithSubscriptionsLatch,
        doneWithExecutionLatch,
        mySubject,
        valuesToLog);

    
    SimTK::Matrix X(parseCSV(xFilename));
    SimTK::Matrix Y(parseCSV(yFilename));
    rtb::Plsr strainPlsr;
    strainPlsr.train(X, Y, 0.001);
    AchillesTendonModel atModel(
        *logQueue["MuscleForces"],
        strainQueue,
        doneWithSubscriptionsLatch,
        doneWithExecutionLatch,
        mySubject.getMuscleNames(),
        strainPlsr);
    
    QueueToCSV strainLogger(
        strainQueue, 
        doneWithSubscriptionsLatch,
        doneWithExecutionLatch, 
        strainoutput);

    //MEGA HACK
    rtb::Concurrency::Latch l1(2);
    rtb::Concurrency::Latch l2(1);
    
    rtosim::TimeDifference<LmtQueue, ScalarVectorQueue> 
        pipelineTimeDifference(
            lmtQueue,
            strainQueue,
            l1,
            l2);

    doneWithSubscriptionsLatch.setCount(8);
    doneWithExecutionLatch.setCount(7);

    auto fixLatches([&]() {
        doneWithSubscriptionsLatch.wait();
        l1.wait(); });

    // 4. start the threads
    std::thread emgProdThread(std::ref(emgProducer));
    std::thread externalTorquesProdThread(std::ref(externalTorquesProducer));
    std::thread lmtMaProdThread(std::ref(lmtMaProducer));
    std::thread simulatorThread(std::ref(simulator));
    std::thread queuesToStorageFilesThread(std::ref(queuesToStorageFiles));
    std::thread atModelThread(std::ref(atModel));
    std::thread strainLoggerThread(std::ref(strainLogger));

    std::thread timeMeasureThread(std::ref(pipelineTimeDifference));
    fixLatches();
    std::cout << "release latches\n";
    emgProdThread.join();
    lmtMaProdThread.join();
    externalTorquesProdThread.join();
    simulatorThread.join();
    queuesToStorageFilesThread.join();
    strainLoggerThread.join();
    atModelThread.join();
    timeMeasureThread.join();

    pipelineTimeDifference.getWallClockDifference().print(outputDirectory + "/wallclock-time-lmtqueue-to-strainqueue.txt");
    pipelineTimeDifference.getCpuClockDifference().print(outputDirectory + "/cupclock-time-lmtqueue-to-strainqueue.txt");
    return 0;
}


SimTK::Matrix parseCSV(const std::string& filename) {

    std::ifstream data(filename);
    std::string line;
    std::vector<std::vector<double>> parsedCsv;
    while (std::getline(data, line))
    {
        std::stringstream lineStream(line);
        std::string cell;
        std::vector<double> parsedRow;
        while (std::getline(lineStream, cell, ','))
        {
            parsedRow.push_back(std::stod(cell));
        }
        parsedCsv.push_back(parsedRow);
    }
    SimTK::Matrix m(parsedCsv.size(), parsedCsv.front().size());
    for (int r(0); r < parsedCsv.size(); ++r) {
        for (int c(0); c < parsedCsv.front().size(); ++c) {
            m(r, c) = parsedCsv[r][c];
        }
    }

    return m;
};

void printCSV(const SimTK::MatrixView m, const std::string& outputFilename) {

    std::string sp(",");
    std::ofstream oF(outputFilename);
    for (int r(0); r < m.nrow(); ++r) {
        for (int c(0); c < m.ncol(); ++c) {
            oF << m(r, c);
            if (c != m.ncol() - 1)
                oF << sp;
        }
        if (r != m.nrow() - 1)
            oF << "\n";
    }
    oF.close();
}

void printCSV(const std::list<std::vector<double>> m, const std::string& outputFilename) {

    std::string sp(",");
    std::ofstream oF(outputFilename);
    for (auto& r : m) {
        for (auto& c : r) {
            oF << c;
            if (&c != &r.back())
                oF << sp;
        }
        if (&r != &m.back())
            oF << "\n";
    }
    oF.close();
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