#include "OpenSim/OpenSim.h"
#include "rtb/concurrency/Concurrency.h"
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
#include "ceinms/EmgFromQueue.h"
#include "ceinms/RawEmgFromFile.h"
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
#include "ceinms/websocket//QueuesToWebsocket.h"

#include "ceinms/queues/MuscleForcesQueue.h"
#include "ceinms/queues/ScalarVectorQueue.h"
#include "ceinms/LmtMaFromMcbs.h"
#include "rtosim/TimeDifference.h"
#include "rtosim/QueueAdapter.h"
#include "rtosim/DataFromNexus.h"
#include "rtosim/InverseKinematics.h"
#include "rtosim/OsimUtilities.h"
#include "rtosim/GeneralisedCoordinatesStateSpace.h"
#include "rtosim/SignalProcessing.h"
#include "rtosim/rtosim.h"
#include <numeric>
#include <vector>
#include <list>
#include <chrono>
#include <functional>
using namespace ceinms;
using rtb::Concurrency::Latch;
using rtb::Concurrency::Queue;

using NMSmodelType = NMSmodel<ExponentialActivation, ElasticTendon_BiSec, CurveMode::Online>;
SimTK::Matrix parseCSV(const std::string& filename);
void printCSV(const SimTK::MatrixView m, const std::string& outputFilename);
void printCSV(const std::list<std::vector<double>> m, const std::string& outputFilename);
void sortMaFilenames(const std::map<std::string, std::string>& maMap, const std::vector< std::string >& dofNames, std::vector< std::string >& maDataFilenames);

struct  ItrainExecutionSetup {
	string osimModelFilename;
	string ceinmsSubject;
	string ikTaskFilename;
	string mcbsDir;
	bool usePlsr;
	string plsrDir;
	bool runFromDevice;
	string hostname;
	bool runFromFile;
	string trcFilename;
	string emgFilename;
	double fc;
	unsigned nThreads;
	double solverAccuracy;
	string resultDir;
	bool showVisualiser;
	string maxEmgFile;
	bool useMaxEmgFile;
	string emgGeneratorFile;
	double pushFrequency;
	double startTime;
	double stopTime;
};
//introduces a time delay in the pipeline

template<typename T>
class Ticker {
public:
	Ticker(unsigned milliseconds) :
		delay_(milliseconds) {}
	T operator()(const T& frame) {
		std::this_thread::sleep_for(std::chrono::milliseconds(delay_));
		std::cout << "." << std::flush;
		return frame;
	}

private:
	unsigned delay_;

};


class AchillesTendonModel {

public:
    AchillesTendonModel(
        MuscleForcesQueue& inputMuscleForcesQueue,
        ScalarVectorVectorQueue& outputStrainQueue,
        Latch& doneWithSubscriptions,
        Latch& doneWithExecution,
        const std::vector<std::string>& allMuscleNames,
        rtb::Plsr strainPls,
		const SimTK::Matrix& nodes) :
        inputMuscleForcesQueue_(inputMuscleForcesQueue),
        outputStrainQueue_(outputStrainQueue),
        doneWithSubscriptions_(doneWithSubscriptions),
        doneWithExecution_(doneWithExecution),
        allMuscleNames_(allMuscleNames),
        strainPls_(strainPls),
		nodes_(nodes)
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
				ScalarVectorVectorFrame outputStrains;
				outputStrains.time = muscleForcesFrame.time;
				outputStrains.data.resize(4);
				for (auto& v : outputStrains.data)
					v.resize(strains.ncol());
				for (size_t c(0); c < nodes_.ncol(); ++c) {
					for (size_t r(0); r < nodes_.nrow(); ++r) {
						outputStrains.data[c][r] = nodes_(r, c);
					}
				}
				for (size_t i(0); i < strains.ncol(); ++i) {
					outputStrains.data.at(3).at(i) = strains(0, i);
				}
				outputStrainQueue_.push(outputStrains);
			}
			else {
				outputStrainQueue_.push(EndOfData::get<ScalarVectorVectorFrame>());
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
		while (&inputMuscleForcesQueue_ == nullptr) {
			//wait to connect to the queue
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}
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
				ScalarVectorVectorFrame outputStrains;
				outputStrains.time = muscleForcesFrame.time;
				outputStrains.data.resize(4);
				for (auto& v : outputStrains.data)
					v.resize(nCols);
				for (size_t c(0); c < nodes_.ncol(); ++c) {
					for (size_t r(0); r < nodes_.nrow(); ++r) {
						outputStrains.data[c][r] = nodes_(r, c);
					}
				}
				for (size_t i(0); i < nCols; ++i) {
                    outputStrains.data.at(3).at(i) = (B[i] * atForce) + meanY[i];
                }
                outputStrainQueue_.push(outputStrains);
            }
            else {
                outputStrainQueue_.push(EndOfData::get<ScalarVectorVectorFrame>());
            }
        }
        doneWithExecution_.wait();
    }


private:
    MuscleForcesQueue& inputMuscleForcesQueue_;
	ScalarVectorVectorQueue& outputStrainQueue_;
    Latch& doneWithSubscriptions_;
    Latch& doneWithExecution_;
    std::vector<std::string> allMuscleNames_;
    rtb::Plsr strainPls_;
	SimTK::Matrix nodes_;
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


class GeneralisedCoordinatesReducer {
private:
	ceinms::Mapper reducer_;
public:
	GeneralisedCoordinatesReducer(vector<string> dofNamesfromOsim, vector<string> dofNamesfromSubject) :
		reducer_(dofNamesfromOsim, dofNamesfromSubject) {}
	ceinms::GeneralisedCoordinatesFrame operator()(const rtosim::GeneralisedCoordinatesFrame& frame) {

		auto reducedQ(reducer_.map(frame.data.getQ()));
		rtosim::GeneralisedCoordinatesData reducedData(reducer_.getOutSize());
		reducedData.setQ(reducedQ);
		return{ frame.time, reducedData };
	}
};

template<typename TIn, typename TOut>
class FixMissingEmgData {
	/* Noraxon sets missing emgs to 10000uV to be easily identifiable. If any emg value at any time is greater than `threshold` we set it to zero,
	otherwise it messes up the filtering*/
private:
	double threshold_;
public:
	FixMissingEmgData(double threshold = 1000) :
		threshold_(threshold) {}
	TOut operator()(const TIn& frame) {
		TOut cFrame;
		cFrame.data = frame.data;
		cFrame.time = frame.time;
		for (auto &e : cFrame.data) {
			if (e > threshold_)
				e = 0.;
		}
		return cFrame;
	}
};

//2 latches
void deviceProducer(
	ceinms::EmgQueue& outputRawEmgQueue,
	rtosim::MarkerSetQueue& outputMarkerQueue,
	rtb::Concurrency::Latch& subscriptionLatch,
	rtb::Concurrency::Latch& endLatch,
	ItrainExecutionSetup itrainSetup) {

	//used to stop the data producers
	rtosim::FlowControl flowCondition(true);
	rtosim::EmgQueue localRawEmgQueue;
	ExcitationGeneratorFromXml emgGenerator(itrainSetup.emgGeneratorFile);
	vector<string> emgChannelNames{ emgGenerator.getInputSignalsNames() };

	rtosim::DataFromNexus dataProducerFromNexus(outputMarkerQueue, localRawEmgQueue, subscriptionLatch, endLatch, flowCondition, itrainSetup.osimModelFilename, emgChannelNames);
	dataProducerFromNexus.setLoopMode(true);
	rtosim::QueueAdapter<
		rtosim::EmgQueue,
		ceinms::EmgQueue,
		FixMissingEmgData<rtosim::EmgQueue::type, ceinms::EmgQueue::type>
	> emgFixer(
			localRawEmgQueue,
			outputRawEmgQueue,
		subscriptionLatch,
		endLatch,
			FixMissingEmgData<rtosim::EmgQueue::type, ceinms::EmgQueue::type >(1000.));

	auto trigger([&flowCondition]() {
		std::string cmd;
		bool run(true);
		std::cin >> cmd;
		flowCondition.setRunCondition(false);
	});


	std::thread dataProducerThread(std::ref(dataProducerFromNexus));
	std::thread emgFixerThread(std::ref(emgFixer));
	std::thread triggerThread(std::ref(trigger));
	
	subscriptionLatch.wait();
	endLatch.wait();

	dataProducerThread.join();
	emgFixerThread.join();
	triggerThread.join();
}

//3 latches
template<typename NMSmodelT>
void fileProducer(
	ceinms::EmgQueue& outputRawEmgQueue,
	rtosim::MarkerSetQueue& outputMarkerQueue,
	rtb::Concurrency::Latch& subscriptionLatch,
	rtb::Concurrency::Latch& endLatch,
	NMSmodelT& subject,
	ItrainExecutionSetup itrainSetup) {

	ceinms::EmgQueue localRawEmgQueue;
	rtosim::MarkersFromTrc markersFromTrc(outputMarkerQueue, subscriptionLatch, endLatch, itrainSetup.osimModelFilename, itrainSetup.trcFilename, false);
	if(itrainSetup.pushFrequency > 0)
		markersFromTrc.setOutputFrequency(itrainSetup.pushFrequency);
	if (itrainSetup.startTime > 0)
		markersFromTrc.setStartTime(itrainSetup.startTime);
	if (itrainSetup.stopTime > 0)
		markersFromTrc.setStopTime(itrainSetup.stopTime);


	ExcitationGeneratorFromXml emgGenerator(itrainSetup.emgGeneratorFile);
	vector<string> desiredEmgChannelNames{ emgGenerator.getInputSignalsNames() };
	ceinms::RawEmgFromFile emgFromFile(localRawEmgQueue, subscriptionLatch, endLatch, itrainSetup.emgFilename, desiredEmgChannelNames);
	rtosim::QueueAdapter<
		ceinms::EmgQueue,
		ceinms::EmgQueue,
		FixMissingEmgData<ceinms::EmgQueue::type, ceinms::EmgQueue::type>> emgFixer(
			localRawEmgQueue,
			outputRawEmgQueue,
			subscriptionLatch,
			endLatch,
			FixMissingEmgData<ceinms::EmgQueue::type, ceinms::EmgQueue::type >(0.009));

	std::thread markerProducerThread(std::ref(markersFromTrc));
	std::thread emgProducerThread(std::ref(emgFromFile));
	std::thread emgFixerThread(std::ref(emgFixer));


	markerProducerThread.join();
	emgProducerThread.join();
	emgFixerThread.join();

}

//7 latches
template<typename NMSmodelT>
void dataConditioning(
	ceinms::EmgQueue& inputRawEmgQueue,
	rtosim::MarkerSetQueue& inputMarkerQueue,
	ceinms::EmgQueue& outputExcitationsQueue,
	ceinms::LmtQueue& outputLmtQueue,
	ceinms::MultipleMomentArmsQueues& outputMomentArmsQueue,
	ceinms::ExternalTorquesQueue& outputTorquesQueue,
	rtb::Concurrency::Latch& subscriptionLatch,
	rtb::Concurrency::Latch& endLatch,
	NMSmodelT& subject,
	ItrainExecutionSetup itrainSetup) {

	ExcitationGeneratorFromXml emgGenerator(itrainSetup.emgGeneratorFile);
	vector<string> emgChannelNames{ emgGenerator.getInputSignalsNames() };
	vector<string> dofNamesFromOsim{ rtosim::getCoordinateNamesFromModel(itrainSetup.osimModelFilename)};
	vector<string> dofNamesFromSubject;
	subject.getDoFNames(dofNamesFromSubject);

	std::unique_ptr<ceinms::EmgFromX> emgFromQueue;
	if (itrainSetup.useMaxEmgFile) {
		emgFromQueue = std::make_unique<ceinms::EmgFromQueue>(
			ceinms::EmgFromQueue(inputRawEmgQueue, outputExcitationsQueue,
				subscriptionLatch, endLatch, itrainSetup.maxEmgFile,
				itrainSetup.emgGeneratorFile, subject, 6., 1500.));
	}
	else {
		emgFromQueue = std::make_unique<ceinms::EmgFromQueue>(
			ceinms::EmgFromQueue(inputRawEmgQueue, outputExcitationsQueue,
				subscriptionLatch, endLatch, emgChannelNames,
				itrainSetup.emgGeneratorFile, subject, 6., 1500.));
	}

	rtosim::GeneralisedCoordinatesQueue generalisedCoordinatesQueue;
	rtosim::QueueToInverseKinametics queueToInverseKinematics(
		inputMarkerQueue, 
		generalisedCoordinatesQueue, 
		subscriptionLatch, endLatch,
		itrainSetup.osimModelFilename, 
		itrainSetup.nThreads, 
		itrainSetup.ikTaskFilename, 1e-5, 10000);

	//Filter generalised coordinated form IK
	rtosim::GeneralisedCoordinatesQueue reducedGeneralisedCoordinatesQueue, filteredGeneralisedCoordinatesQueue;
	rtosim::GeneralisedCoordinatesStateSpace gcFilt(itrainSetup.fc, dofNamesFromOsim.size());
	rtosim::QueueAdapter <
		rtosim::GeneralisedCoordinatesQueue,
		rtosim::GeneralisedCoordinatesQueue,
		rtosim::GeneralisedCoordinatesStateSpace
	> gcQueueAdaptor(
		generalisedCoordinatesQueue,
		filteredGeneralisedCoordinatesQueue,
		subscriptionLatch,
		endLatch,
		gcFilt);
	//extract a subset of coordinates to be used with MCBS
	rtosim::QueueAdapter<rtosim::GeneralisedCoordinatesQueue, rtosim::GeneralisedCoordinatesQueue, GeneralisedCoordinatesReducer> gcReducer(
		filteredGeneralisedCoordinatesQueue,
		reducedGeneralisedCoordinatesQueue,
		subscriptionLatch,
		endLatch,
		GeneralisedCoordinatesReducer(dofNamesFromOsim, dofNamesFromSubject));

	ceinms::LmtMaFromMcbs lmtMaFromMcbs(
		reducedGeneralisedCoordinatesQueue,
		outputLmtQueue,
		outputMomentArmsQueue,
		subscriptionLatch,
		endLatch,
		subject,
		itrainSetup.mcbsDir,
		itrainSetup.osimModelFilename);

	rtosim::QueueToFileLogger<rtosim::GeneralisedCoordinatesData> filteredIkLogger(
		filteredGeneralisedCoordinatesQueue,
		subscriptionLatch,
		endLatch,
		dofNamesFromOsim,
		itrainSetup.resultDir, "filtered_ik", ".sto");

	rtosim::StateVisualiser visualizer(generalisedCoordinatesQueue, itrainSetup.osimModelFilename);
	
	if (itrainSetup.showVisualiser) {
		std::thread t(std::ref(visualizer));
		t.detach();
	}
	std::thread emgFromQueueThread(std::ref(*emgFromQueue));
	std::thread inverseKinematicsThread(std::ref(queueToInverseKinematics));
	std::thread gcFiltThread(std::ref(gcQueueAdaptor));
	std::thread gcReducerThread(std::ref(gcReducer));
	std::thread lmtMaFromMcbsThread(std::ref(lmtMaFromMcbs));
	std::thread filteredIkLoggerThread(std::ref(filteredIkLogger));

	subscriptionLatch.wait();

	//signal to all consumers of torque data that there is no torque data
	//so they don't hang waiting for it. If we had a torque producer of some sort, 
	//such as a direct measurement from and exoskeleton, or the output of the inverse dynamics,
	//then we would have thread sending data to the TorquesQueue.
	auto eod = EndOfData::get<ExternalTorquesFrame>();
	outputTorquesQueue.push(eod);

	endLatch.wait();

	emgFromQueueThread.join();
	inverseKinematicsThread.join();
	gcFiltThread.join();
	gcReducerThread.join();
	lmtMaFromMcbsThread.join();
	filteredIkLoggerThread.join();
}

// 2 latches
template<typename NMSmodelT>
void ceinmsProcessing(
	ceinms::EmgQueue& inputExcitationQueue,
	ceinms::LmtQueue& inputLmtQueue,
	ceinms::MultipleMomentArmsQueues& inputMomentArmsQueue,
	ceinms::ExternalTorquesQueue& inputExternalTorquesQueue,
	ceinms::ScalarLogQueue& outputDataQueue,
	rtb::Concurrency::Latch& doneWithSubscriptionsLatch,
	rtb::Concurrency::Latch& doneWithExecutionLatch,
	NMSmodelT& subject,
	ItrainExecutionSetup itrainSetup) {
		
	vector<string> dataToLog = {
		"Activations",
		"FibreLengths",
		"NormFibreLengths",
		"FibreVelocities",
		"NormFibreVelocities",
		"PennationAngles",
		"MuscleForces",
		"Torques",
		"TendonLengths",
		"NormTendonLengths",
		"MusclesContribution" };
	// 2b. define the thread consuming the output sources
	vector<string> valuesToWrite = dataToLog;
	ceinms::initialiseQueue(outputDataQueue, valuesToWrite);
	// 3. define the model simulator
	vector<string> valuesToLog = dataToLog;
	ModelEvaluationOnline<NMSmodelT, LoggerOnQueues>  simulator(
		inputExcitationQueue,
		inputLmtQueue,
		inputExternalTorquesQueue,
		inputMomentArmsQueue,
		outputDataQueue,
		doneWithSubscriptionsLatch,
		doneWithExecutionLatch,
		subject,
		valuesToLog);

	QueuesToStorageFiles queuesToStorageFiles(
		outputDataQueue,
		doneWithSubscriptionsLatch, 
		doneWithExecutionLatch, 
		subject,
		valuesToWrite, 
		itrainSetup.resultDir);


	std::thread simulatorThread(std::ref(simulator));
	std::thread loggingThread(std::ref(queuesToStorageFiles));
	
	simulatorThread.join();
	loggingThread.join();
}


//3 latches
template<typename NMSmodelT>
void surrogateModelProcessing(
	ceinms::ScalarLogQueue& inputDataQueue, 
	ceinms::ScalarVectorVectorQueue& outputstrainQueue, 
	rtb::Concurrency::Latch& subscriptionLatch,
	rtb::Concurrency::Latch& endLatch, 
	NMSmodelT& subject, ItrainExecutionSetup itrainSetup) {



	string plsrXfile{ itrainSetup.plsrDir + "/xvalues.csv" };
	string plsrYfile{ itrainSetup.plsrDir + "/yvalues.csv" };
	string plsrNodeFile{ itrainSetup.plsrDir + "/nodes.csv" };
	SimTK::Matrix X(parseCSV(plsrXfile));
	SimTK::Matrix Y(parseCSV(plsrYfile));
	rtb::Plsr strainPlsr;
	strainPlsr.train(X, Y, 0.001);
	SimTK::Matrix nodes(parseCSV(plsrNodeFile));

	AchillesTendonModel atModel(
		*inputDataQueue["MuscleForces"],
		outputstrainQueue,
		subscriptionLatch,
		endLatch,
		subject.getMuscleNames(),
		strainPlsr,
		nodes);

	ceinms::ScalarVectorQueue reducedStrainQueue;
	rtosim::QueueAdapter<
		ScalarVectorVectorQueue, 
		ScalarVectorQueue, 
		std::function<ScalarVectorFrame(const ScalarVectorVectorFrame&)>> strainReducerAdapter(
			outputstrainQueue,
			reducedStrainQueue,
			subscriptionLatch,
			endLatch,
			[](const ScalarVectorVectorFrame& frame) {
		//Strain values are stored in the last columns
		ScalarVectorFrame strainFrame{ frame.time, frame.data.at(3) };
		return strainFrame;
		}
	);

	string strainOutput(itrainSetup.resultDir + "/strain_fe.csv");
	QueueToCSV strainLogger(
		reducedStrainQueue,
		subscriptionLatch,
		endLatch,
		strainOutput);

	ceinms::QueuesToWebsocket queueToWebsocket(outputstrainQueue, { "nodes_x","nodes_y","nodes_z","nodes_w" });

	std::thread atModelThread(std::ref(atModel));
	std::thread queueToWebsocketThread(std::ref(queueToWebsocket));
	std::thread strainLoggerThread(std::ref(strainLogger));
	std::thread strainReducerAdapterThread(std::ref(strainReducerAdapter));
	
	queueToWebsocketThread.detach();
	atModelThread.join();
	strainLoggerThread.join();
	strainReducerAdapterThread.join();
}

int run_from_file(ItrainExecutionSetup itrainSetup) {


	string baseDir = "C:\\Data\\ABC11\\";
    string subjectFile = baseDir + "TestData\\20151214\\ceinms\\subjects\\calibrated\\calibrated0.xml";
    string trialFilename = baseDir + "TestData\\20151214\\ceinms\\trials\\baseline_walking.xml";
    string emgGeneratorFile = baseDir + "TestData\\20151214\\ceinms\\excitationGenerator\\excitationGenerator16to34-RIGHT.xml";
    string xFilename(baseDir + "analysis\\strainX.csv");
    string yFilename(baseDir + "analysis\\strainY.csv");
    string strainoutput(baseDir + "output\\strain.csv");
    string outputDirectory(baseDir + "output");
	string nodesFilename = "C:\\Users\\s2849511\\coding\\unversioned\\boostwebsockettest\\data\\nodes.csv";
//			std::string strainsFilename = "C:\\Users\\s2849511\\coding\\unversioned\\boostwebsockettest\\data\\strains.csv";
    
    InputDataXmlReader dataLocations(trialFilename);

    //Create queues for input data
    EmgQueue emgQueue;
    LmtQueue lmtQueue, delayedLmtQueue;
    ExternalTorquesQueue externalTorqueQueue;
    MultipleMomentArmsQueues momentArmsQueues;

    //Creates queues for output data
    ScalarLogQueue logQueue;
    ScalarVectorVectorQueue strainQueue;

    //Creates latches to sync threads
    Latch doneWithSubscriptionsLatch;
    Latch doneWithExecutionLatch;
    
    NMSmodelType mySubject;
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
		"TendonLengths",
		"NormTendonLengths",
		"MusclesContribution" };
    // 2b. define the thread consuming the output sources
    vector<string> valuesToWrite = dataToLog;
    QueuesToStorageFiles queuesToStorageFiles(logQueue, doneWithSubscriptionsLatch, doneWithExecutionLatch, mySubject, valuesToWrite, outputDirectory);

    // 3. define the model simulator
    vector<string> valuesToLog = dataToLog;
    ModelEvaluationOnline<NMSmodelType, LoggerOnQueues>  simulator(
        emgQueue,
		delayedLmtQueue,
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
	SimTK::Matrix nodes(parseCSV(nodesFilename));

    AchillesTendonModel atModel(
        *logQueue["MuscleForces"],
        strainQueue,
        doneWithSubscriptionsLatch,
        doneWithExecutionLatch,
        mySubject.getMuscleNames(),
        strainPlsr,
		nodes);
    
//    QueueToCSV strainLogger(
//        strainQueue, 
//        doneWithSubscriptionsLatch,
//        doneWithExecutionLatch, 
//        strainoutput);

    //MEGA HACK
    rtb::Concurrency::Latch l1(3);
    rtb::Concurrency::Latch l2(2);
    
	Ticker<LmtQueue::type> ticker(5);
	rtosim::QueueAdapter< LmtQueue, LmtQueue, Ticker<LmtQueue::type>> delayer(lmtQueue, delayedLmtQueue,l1, l2, ticker);

    rtosim::TimeDifference<LmtQueue, ScalarVectorVectorQueue> 
        pipelineTimeDifference(
            lmtQueue,
            strainQueue,
            l1,
            l2);
	ceinms::QueuesToWebsocket queueToWebsocket(strainQueue, { "nodes_x","nodes_y","nodes_z","nodes_w" }, "132.234.133.234", "8080");
    doneWithSubscriptionsLatch.setCount(6);
    doneWithExecutionLatch.setCount(5);

    auto fixLatches([&]() {
        doneWithSubscriptionsLatch.wait();
        l1.wait(); });

    // 4. start the threads
    std::thread emgProdThread(std::ref(emgProducer));
    std::thread externalTorquesProdThread(std::ref(externalTorquesProducer));
    std::thread lmtMaProdThread(std::ref(lmtMaProducer));
    std::thread simulatorThread(std::ref(simulator));
 //   std::thread queuesToStorageFilesThread(std::ref(queuesToStorageFiles));
    std::thread atModelThread(std::ref(atModel));
 //   std::thread strainLoggerThread(std::ref(strainLogger));
	std::thread delayerThread(std::ref(delayer));
    std::thread timeMeasureThread(std::ref(pipelineTimeDifference));
	std::thread queueToWebsocketThread(std::ref(queueToWebsocket));
    fixLatches();
    std::cout << "release latches\n";

    emgProdThread.join();
    lmtMaProdThread.join();
    externalTorquesProdThread.join();
    simulatorThread.join();
 //   queuesToStorageFilesThread.join();
 //   strainLoggerThread.join();
    atModelThread.join();
    timeMeasureThread.join();
	delayerThread.join();
	queueToWebsocketThread.join();
    pipelineTimeDifference.getWallClockDifference().print(outputDirectory + "/wallclock-time-lmtqueue-to-strainqueue.txt");
    pipelineTimeDifference.getCpuClockDifference().print(outputDirectory + "/cupclock-time-lmtqueue-to-strainqueue.txt");
    return 0;
}


int run(ItrainExecutionSetup itrainSetup) {
	
	
	//Creates latches to sync threads
	Latch doneWithSubscriptionsLatch(17);
	Latch doneWithExecutionLatch(15);

	NMSmodelType mySubject;
	setupSubject(mySubject, itrainSetup.ceinmsSubject);
	vector<string> dofNamesFromOsim, dofNamesFromSubject;
	mySubject.getDoFNames(dofNamesFromSubject);

	//Create data queues 
	ceinms::EmgQueue rawEmgQueue, excitationsQueue;
	rtosim::MarkerSetQueue markerSetQueue;
	ceinms::LmtQueue lmtQueue;
	ceinms::ExternalTorquesQueue externalTorqueQueue;
	ceinms::MultipleMomentArmsQueues momentArmsQueues;

	//Creates queues for output data
	ceinms::ScalarLogQueue ceinmsOutputQueue;
	ceinms::ScalarVectorVectorQueue strainQueue;
		
	
	auto onScreenDisplay([&]() {
		doneWithSubscriptionsLatch.wait();
		momentArmsQueues[0]->subscribe();
		bool  runcondition = true;
		int counter{ 100 };
		while (runcondition) {
			auto f = momentArmsQueues[0]->pop();
			runcondition = !EndOfData::isEod(f);
			if (--counter < 0) {
				counter = 100;
				std::cout << "." << std::flush;
			}
		}
	});
		
	std::thread onScreenDisplayThread(std::ref(onScreenDisplay));
	std::thread producerThread;

	if (itrainSetup.runFromDevice) {
		producerThread = std::thread(
			deviceProducer,
			std::ref(rawEmgQueue),
			std::ref(markerSetQueue),
			std::ref(doneWithSubscriptionsLatch),
			std::ref(doneWithExecutionLatch),
			itrainSetup);
	}
	else
	{
		doneWithSubscriptionsLatch.increaseCount(1);
		doneWithExecutionLatch.increaseCount(1);
		producerThread = std::thread(
			fileProducer<NMSmodelType>,
			std::ref(rawEmgQueue),
			std::ref(markerSetQueue),
			std::ref(doneWithSubscriptionsLatch),
			std::ref(doneWithExecutionLatch),
			std::ref(mySubject),
			itrainSetup);
	}
	
	std::thread dataConditioningThread(
		dataConditioning<NMSmodelType>,
		std::ref(rawEmgQueue),
		std::ref(markerSetQueue),
		std::ref(excitationsQueue),
		std::ref(lmtQueue),
		std::ref(momentArmsQueues),
		std::ref(externalTorqueQueue),
		std::ref(doneWithSubscriptionsLatch),
		std::ref(doneWithExecutionLatch),
		std::ref(mySubject),
		itrainSetup);
	
	std::thread ceinmsProcessingThread(
		ceinmsProcessing<NMSmodelType>,
		std::ref(excitationsQueue),
		std::ref(lmtQueue),
		std::ref(momentArmsQueues),
		std::ref(externalTorqueQueue),
		std::ref(ceinmsOutputQueue),
		std::ref(doneWithSubscriptionsLatch),
		std::ref(doneWithExecutionLatch),
		std::ref(mySubject),
		itrainSetup);

	std::thread surrogateModelProcessingThread(
		surrogateModelProcessing<NMSmodelType>,
		std::ref(ceinmsOutputQueue),
		std::ref(strainQueue),
		std::ref(doneWithSubscriptionsLatch),
		std::ref(doneWithExecutionLatch),
		mySubject,
		itrainSetup);
		
	rtosim::TimeDifference<rtosim::MarkerSetQueue, ceinms::ScalarVectorVectorQueue>
		pipelineTimeDifference(markerSetQueue, strainQueue, doneWithSubscriptionsLatch, doneWithExecutionLatch);
	rtosim::TimeProbe< ceinms::ScalarVectorVectorQueue> timeProbe(strainQueue, doneWithSubscriptionsLatch);


	std::thread timeDifferenceThread(std::ref(pipelineTimeDifference));
	std::thread timeProbeThread(std::ref(timeProbe));
	onScreenDisplayThread.join();
	producerThread.join();
	dataConditioningThread.join();
	ceinmsProcessingThread.join();
	surrogateModelProcessingThread.join();
	timeDifferenceThread.join();
	timeProbeThread.join();

	pipelineTimeDifference.getWallClockDifference().print(itrainSetup.resultDir + "/wallclock-time-markerqueue-to-strainqueue.txt");
	pipelineTimeDifference.getCpuClockDifference().print(itrainSetup.resultDir + "/cupclock-time-markerqueue-to-strainqueue.txt");
	auto times = pipelineTimeDifference.getWallClockDifference().getTimes();
	std::ofstream outF(itrainSetup.resultDir + "/wallclock-times.txt");
	for (auto& e : times)
		outF << e << '\n';
	std::cout << "Total execution time: " << timeProbe.getWallClockTimes().getMax() - timeProbe.getWallClockTimes().getMin() << std::endl;
	std::cout << "Number of frames:     " << timeProbe.getWallClockTimes().getTimes().size() << std::endl;
	return 0;
}

void test_visualiser(ItrainExecutionSetup setup) {

	rtosim::GeneralisedCoordinatesQueue queue;
	OpenSim::Model model(setup.osimModelFilename);
	model.setUseVisualizer(true);
	SimTK::State s(model.initSystem());
	SimTK::Visualizer &viz = model.updVisualizer().updSimbodyVisualizer();

	//     viz.setShowFrameRate(true);
	  //   viz.setShutdownWhenDestructed(true);
	 //    viz.setMode(SimTK::Visualizer::Mode::Sampling);
	//     viz.setShutdownWhenDestructed(true);
		// viz.setDesiredBufferLengthInSec(5);
	   //  viz.setDesiredFrameRate(60);
	auto start = std::chrono::steady_clock::now();
	int frameCounter = 0;
	model.getVisualizer().show(s);
	getchar();
}

void printAuthors() {

	cout << "Real-time Achilles tendon strain" << endl;
	cout << "Authors: Claudio Pizzolato <c.pizzolato@griffith.edu.au>" << endl << endl;
}

void printHelp(ItrainExecutionSetup itrainSetup) {

	printAuthors();

	auto progName(SimTK::Pathname::getThisExecutablePath());
	bool isAbsolute;
	string dir, filename, ext;
	SimTK::Pathname::deconstructPathname(progName, isAbsolute, dir, filename, ext);

	if (!(itrainSetup.runFromFile || itrainSetup.runFromDevice)) {
		cout << "To run from a connected device use: " << filename << " --device [OPTIONS]\n" 
			"| To run from file use: " << filename << " --file [OPTIONS]\n";
	}
	else {
		if (itrainSetup.runFromDevice) {
			cout << "Option                Argument         Action / Notes\n";
			cout << "------                --------         --------------\n";
			cout << "-h                                     Print the command-line options for " << filename << ".\n";
			cout << "--model               ModelFilename    Specify the name of the osim model file for the investigation.\n";
			cout << "--subject             CEINMS subject   Specify the name of the CEINMS subject file for the investigation.\n";
			cout << "--task-set            TaskSetFilename  Specify the name of the XML TaskSet file containing the marker weights to be used.\n";
			cout << "--plsr                PLSR directory   Specify the name of the directory containing the Achilles tendon PLSR files to be used.\n";
			cout << "--mcbs                MCBS directory   Specify the name of the directory containing the MCBS coefficient files to be used.\n";
			cout << "--maxemg              max EMG file     Specify the name of the file containing the max values for EMG envelopes.\n";
			cout << "--gen                 EMG generator    Specify the name of the file for the EMG generator.\n";
			cout << "--hostname            HostName         Specify IP address and port for Vicon Nexus.\n";
			cout << "--fc                  CutoffFrequency  Specify the name of lowpass cutoff frequency to filter IK and GRF data.\n";
			cout << "-j                    IK threads       Specify the number of IK threads to be used.\n";
			cout << "-a                    Accuracy         Specify the IK solver accuracy.\n";
			cout << "--output              OutputDir        Specify the output directory.\n";
			cout << "-v                                     Show visualiser.\n";
		} if (itrainSetup.runFromFile) {
			cout << "Option                Argument         Action / Notes\n";
			cout << "------                --------         --------------\n";
			cout << "-h                                     Print the command-line options for " << filename << ".\n";
			cout << "--model               ModelFilename    Specify the name of the osim model file for the investigation.\n";
			cout << "--subject             CEINMS subject   Specify the name of the CEINMS subject file for the investigation.\n";
			cout << "--task-set            TaskSetFilename  Specify the name of the XML TaskSet file containing the marker weights to be used.\n";
			cout << "--plsr                PLSR directory   Specify the name of the directory containing the Achilles tendon PLSR files to be used.\n";
			cout << "--mcbs                MCBS directory   Specify the name of the directory containing the MCBS coefficient files to be used.\n";
			cout << "--maxemg              max EMG file     Specify the name of the file containing the max values for EMG envelopes.\n";
			cout << "--emg                 EMG file         Specify the name of the file containing the raw EMG data.\n";
			cout << "--gen                 EMG generator    Specify the name of the file for the EMG generator.\n";
			cout << "--trc                 Trc filename     Specify the name of the file containing the marker data.\n";
			cout << "--fc                  CutoffFrequency  Specify the name of lowpass cutoff frequency to filter IK and GRF data.\n";
			cout << "-j                    IK threads       Specify the number of IK threads to be used.\n";
			cout << "-a                    Accuracy         Specify the IK solver accuracy.\n";
			cout << "--push-frequency      Frequency        Specify at what frequency the file data should be produced.\n";
			cout << "--start               Time             Specify start time for the data streaming.\n";
			cout << "--stop                Time             Specify stop time for the data streaming.\n";
			cout << "--output              OutputDir        Specify the output directory.\n";
			cout << "-v                                     Show visualiser.\n";

		}
	}
		bool runFromDevice;
	string trialXmlFilename;
	bool runFromFile;
}

int main(int argc, char* argv[]) {

	
	ItrainExecutionSetup itrainSetup;

	rtosim::ProgramOptionsParser po(argc, argv);
	if (po.exists("-h") || po.empty()) {
		printHelp(itrainSetup);
		exit(EXIT_SUCCESS);
	}
	
	if (po.exists("--file")) {
		itrainSetup.runFromFile = true;
		itrainSetup.runFromDevice = false;
	}
	else if (po.exists("--device")) {
		itrainSetup.runFromFile = false;
		itrainSetup.runFromDevice = true;
	}
	else {
		printHelp(itrainSetup);
		exit(EXIT_SUCCESS);
	}
	
	if (po.exists("--model"))
		itrainSetup.osimModelFilename = po.getParameter("--model");
	else {
		cout << "Please provide --model option\n";
		printHelp(itrainSetup);
		exit(EXIT_SUCCESS);
	}

	if (po.exists("--task-set"))
		itrainSetup.ikTaskFilename = po.getParameter("--task-set");
	else {
		cout << "Please provide --task-set option\n";
		printHelp(itrainSetup);
		exit(EXIT_SUCCESS);
	}

	if (po.exists("--subject"))
		itrainSetup.ceinmsSubject = po.getParameter("--subject");
	else {
		cout << "Please provide --subject option\n";
		printHelp(itrainSetup);
		exit(EXIT_SUCCESS);
	}

	if (po.exists("--mcbs"))
		itrainSetup.mcbsDir = po.getParameter("--mcbs");
	else {
		cout << "Please provide --mcbs option\n";
		printHelp(itrainSetup);
		exit(EXIT_SUCCESS);
	}

	if (po.exists("--gen"))
		itrainSetup.emgGeneratorFile = po.getParameter("--gen");
	else {
		cout << "Please provide --gen option\n";
		printHelp(itrainSetup);
		exit(EXIT_SUCCESS);
	}
	
	string plsrDir("");
	if (po.exists("--plsr")) {
		itrainSetup.plsrDir = po.getParameter("--plsr");
		itrainSetup.usePlsr = true;
	}
	else {
		itrainSetup.usePlsr = false;
	}
	
	itrainSetup.hostname = "127.0.0.1:801";
	if (po.exists("--hostname"))
		itrainSetup.hostname = po.getParameter("--hostname");

	itrainSetup.fc = 8;
	if (po.exists("--fc"))
		itrainSetup.fc = po.getParameter<double>("--fc");

	itrainSetup.nThreads = 1;
	if (po.exists("-j"))
		itrainSetup.nThreads = po.getParameter<unsigned>("-j");

	itrainSetup.solverAccuracy = 1e-5;
	if (po.exists("-a"))
		itrainSetup.solverAccuracy = po.getParameter<double>("-a");

	itrainSetup.resultDir = "Output";
	if (po.exists("--output"))
		itrainSetup.resultDir = po.getParameter("--output");

	itrainSetup.showVisualiser = po.exists("-v");

	if (po.exists("--maxemg"))
		itrainSetup.maxEmgFile = po.getParameter("--maxemg");

	if (po.exists("--emg"))
		itrainSetup.emgFilename = po.getParameter("--emg");

	if (po.exists("--trc"))
		itrainSetup.trcFilename = po.getParameter("--trc");

	itrainSetup.pushFrequency = -1;
	if (po.exists("--push-frequency"))
		itrainSetup.pushFrequency = po.getParameter<double>("--push-frequency");

	itrainSetup.startTime = -1;
	if (po.exists("--start"))
		itrainSetup.startTime = po.getParameter<double>("--start");
	itrainSetup.stopTime= -1;
	if (po.exists("--stop"))
		itrainSetup.stopTime = po.getParameter<double>("--stop");

	/*string baseDir = "C:\\Data\\ABC11\\";
	string subjectFile = baseDir + "TestData\\20151214\\ceinms\\subjects\\calibrated\\calibrated0.xml";
	//string emgGeneratorFile = baseDir + "TestData\\20151214\\ceinms\\excitationGenerator\\excitationGenerator16to34-RIGHT.xml";
	string emgGeneratorFile = baseDir + "TestData\\20151214\\ceinms\\excitationGenerator\\excitationGenerator16to34-RIGHT-NEXUS.xml";
	string xFilename(baseDir + "analysis\\strainX.csv");
	string yFilename(baseDir + "analysis\\strainY.csv");
	string strainoutput(baseDir + "output\\strain.csv");
	string outputDirectory(baseDir + "output");
	string nodesFilename = "C:\\Users\\s2849511\\coding\\unversioned\\boostwebsockettest\\data\\nodes.csv";
	string osimModelFilename = baseDir + "TestData\\20151214\\models\\2392_scaled_clusters.osim";
	string mcbsDir = baseDir + "TestData\\20151214\\ceinms\\mcbs\\coefficients\\";
	string ikTaskSet = baseDir + "TestData\\20151214\\inverseKinematics\\01\\baseline_walking\\Setup\\iktasksets.xml";
	string maxEmgFile = baseDir + "TestData\\20151214\\maxemg.txt";
	*/
//	test_visualiser(itrainSetup);

	run(itrainSetup);
	return 0;
}

int main_() {


	//run_from_nexus();
	//run_from_file();
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