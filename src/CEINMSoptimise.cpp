/* -------------------------------------------------------------------------- *
* CEINMS is a standalone toolbox for neuromusculoskeletal modelling and      *
* simulation. CEINMS can also be used as a plugin for OpenSim either         *
* through the OpenSim GUI or API. See https://simtk.org/home/ceinms and the  *
* NOTICE file for more information. CEINMS development was coordinated       *
* through Griffith University and supported by the Australian National       *
* Health and Medical Research Council (NHMRC), the US National Institutes of *
* Health (NIH), and the European Union Framework Programme 7 (EU FP7). Also  *
* see the PROJECTS file for more information about the funding projects.     *
*                                                                            *
* Copyright (c) 2010-2015 Griffith University and the Contributors           *
*                                                                            *
* CEINMS Contributors: C. Pizzolato, M. Reggiani, M. Sartori,                *
*                      E. Ceseracciu, and D.G. Lloyd                         *
*                                                                            *
* Author(s): C. Pizzolato, M. Reggiani, E. Ceseracciu                        *
*                                                                            *
* CEINMS is licensed under the Apache License, Version 2.0 (the "License").  *
* You may not use this file except in compliance with the License. You may   *
* obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0.*
*                                                                            *
* Unless required by applicable law or agreed to in writing, software        *
* distributed under the License is distributed on an "AS IS" BASIS,          *
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.   *
* See the License for the specific language governing permissions and        *
* limitations under the License.                                             *
* -------------------------------------------------------------------------- */

#include "ceinms/Utilities.h"
#include "ceinms/FileUtils.h"
#include "ceinms/CeinmsSetupXmlReader.h"
#include "SimulationManager.h"
#include <ctime>
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

//Hybrid optimiser
#include "ceinms/ErrorMinimizerAnnealing.h"
#include "ceinms/HybridWeightings.h"

#include "ceinms/LoggerOnQueues.h"
#include "ceinms/QueuesToTrialData.h"
#include "ceinms/QueueToStorageFile.h"
#include "ceinms/QueuesToStorageFiles.h"
#include "ceinms/QueuesToJointReactionForces.h"
#include "ceinms/QueuesToIntersegmentalAndJointReactionForces.h"
#include "ceinms/QueuesToInverseDynamics.h"
#include "ceinms/QueuesToKneeBalanceContactForces.h"

#include "ceinms/concurrency/Latch.h"
#include "ceinms/concurrency/Queue.h"
#include "ceinms/queues/VecContactForcesQueue.h"
#include "ceinms/queues/ScalarContactForcesQueue.h"
#include "ceinms/queues/EmgQueue.h"
#include "ceinms/queues/LmtQueue.h"
#include "ceinms/queues/ExternalTorquesQueue.h"
#include "ceinms/queues/MultipleMomentArmsQueues.h"
#include "ceinms/queues/LogQueue.h"
#include "ceinms/QueueToStorageFile.h"
#include "ceinms/ArrayConverter.h"
#include "ceinms/TrialData.h"
#include "ceinms/Hybrid/NMSmodelFrameOptimisation.h"
#include "ceinms/BatchEvaluator.h"
#include "SimulationManager.h"
#include "ceinms/SimulatedAnnealing.h"
#include "ceinms/FileUtils.h"
#include <numeric>
#include <algorithm>
#include <string>
using std::string;
#include <iostream>
using std::cout;
using std::endl;


#include <iomanip>

#include <string>
using std::string;
#include <iostream>
using std::cout;
using std::endl;
#include <vector>
using std::vector;
#include <map>
using std::map;
#include <stdlib.h>
#include <algorithm>

#include <future>

void printHeader() {

    cout << endl;
    cout << "+-+-+-+-+-+-+\n"
        << "|C|E|I|N|M|S|\n"
        << "+-+-+-+-+-+-+-+-+-+-+\n"
        << "|C|a|l|i|b|r|a|t|e|d|\n"
        << "+-+-+-+-+-+-+-+-+-+-+-+-+\n"
        << "|E|M|G|-|I|n|f|o|r|m|e|d|\n"
        << "+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+\n"
        << "|N|e|u|r|o|m|u|s|c|u|l|o|s|k|e|l|e|t|a|l|\n"
        << "+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+\n"
        << "|T|o|o|l|b|o|x|\n"
        << "+-+-+-+-+-+-+-+\n";
    string progName = "CEINMSoptimise";
    string version =
        string(CEINMS_MAJOR_VERSION) + "." +
        string(CEINMS_MINOR_VERSION) + "." +
        string(CEINMS_PATCH_VERSION);
    cout << "\n" << progName << " version " << version << "\n";

}

void printAuthors() {

    time_t now = std::time(0);
    tm *gmtm = std::gmtime(&now);
    cout << "Copyright (C) " << gmtm->tm_year + 1900 << endl;
    cout << "Claudio Pizzolato, Monica Reggiani, Massimo Sartori, David Lloyd\n";
    cout << "Software developers: Claudio Pizzolato, Elena Ceseracciu, Monica Reggiani\n";
}

void PrintUsage()
{
    string progName = "CEINMSoptimise";
    cout << "Option            Argument          Action / Notes\n";
    cout << "------            --------          --------------\n";
    cout << "-Help, -H                           Print the command-line options for " << progName << ".\n";
    cout << "-PrintSetup, -PS                    Generates a template Setup file\n";
    cout << "-Setup, -S        SetupFileName     Specify an xml setup file.\n";

}


template<typename NMSmodelT>
ceinms::TrialData readTrialData(const string& inputDataFilename, NMSmodelT& mySubject, const string& emgGeneratorFile) {

    ceinms::InputDataXmlReader dataLocations(inputDataFilename);
    ceinms::EmgQueue emgQueue;
    ceinms::LmtQueue lmtQueue;
    ceinms::ExternalTorquesQueue externalTorqueQueue;
    ceinms::MultipleMomentArmsQueues momentArmsQueues;

    ceinms::Concurrency::Latch doneWithSubscription;
    ceinms::Concurrency::Latch doneWithExecution;

    string emgFilename(dataLocations.getExcitationsFile());
    ceinms::EmgFromFile emgProducer(emgQueue, doneWithSubscription, doneWithExecution, mySubject, emgFilename, emgGeneratorFile);

    vector< string > dofNames;
    mySubject.getDoFNames(dofNames);
    vector< string > maFilename;
    ceinms::FileUtils::sortMaFilenames(dataLocations.getMaFiles(), dofNames, maFilename);
    ceinms::LmtMaFromStorageFile lmtMaProducer(lmtQueue, momentArmsQueues, doneWithSubscription, doneWithExecution, mySubject, dataLocations.getLmtFile(), maFilename);

    string externalTorqueFilename(dataLocations.getExternalTorqueFile());
    ceinms::ExternalTorquesFromStorageFile externalTorquesProducer(externalTorqueQueue, doneWithSubscription, doneWithExecution, mySubject, externalTorqueFilename);

    ceinms::QueuesToTrialData queuesToTrialData(emgQueue, lmtQueue, externalTorqueQueue, momentArmsQueues, doneWithSubscription, doneWithExecution, mySubject, inputDataFilename);

    doneWithSubscription.setCount(4);
    doneWithExecution.setCount(4);

    // 4. start the threads
    std::thread emgProdThread(std::ref(emgProducer));
    std::thread externalTorquesProdThread(std::ref(externalTorquesProducer));
    std::thread lmtMaProdThread(std::ref(lmtMaProducer));
    std::thread queuesToTrialDataThread(std::ref(queuesToTrialData));

    emgProdThread.join();
    lmtMaProdThread.join();
    externalTorquesProdThread.join();
    queuesToTrialDataThread.join();

    if (!queuesToTrialData.getTrialData().isConsistent())
    {
        std::cout << "ERROR: No data was read for trial "  << std::endl;
        exit(EXIT_FAILURE);
    }

    ceinms::TrialData trial(queuesToTrialData.getTrialData());
    if (mySubject.hasOpenSimModel())
        trial.setOpenSimData(mySubject.getOpenSimModelFilename(), dataLocations.getExternalLoadsFile(), dataLocations.getMotionFile());
    return trial;
}



class HybridOptimisationManager {

public:
    HybridOptimisationManager(const std::string& setupFilename):
        setupFilename_(setupFilename),
        ceinmsSetup_(setupFilename),
        dataLocations_(ceinmsSetup_.getInputDataFile()),
        executionCfg_(ceinmsSetup_.getExecutionFile()),
        subjectFile_(ceinmsSetup_.getSubjectFile()),
        outputDirectory_(ceinmsSetup_.getOutputDirectory()),
        emgGeneratorFile_(ceinmsSetup_.getExcitationGeneratorFile()) {    
    }

    bool run() {
        if (executionCfg_.isHybrid())
            runWeightingOptimisation();
        else
            runOpenLoop();
        return true;
    }
private:
    struct HybridErrors {
        double beta, gamma;
        double trackError, torqueError;
    };

    bool runOpenLoop() {
        ceinms::SimulationManager simulationManager(setupFilename_);
        return simulationManager.run();
    }

    bool runWeightingOptimisation() {

        using NMSmodelT = ceinms::NMSmodel<ceinms::ExponentialActivation, ceinms::ElasticTendon_BiSec, ceinms::CurveMode::Online>;
        NMSmodelT subject;
        setupSubject(subject, subjectFile_, executionCfg_.getTolerance());
        ceinms::TrialData trial(readTrialData(ceinmsSetup_.getInputDataFile(), subject, emgGeneratorFile_));
        
        auto f([=](double beta, double gamma, NMSmodelT subject, ceinms::TrialData trial) {
            int nMtu = subject.getNoMuscles();
            int nDofs = subject.getNoDoF();
            ceinms::Result results(nMtu, nDofs, trial.getEmgData().getNRows(), trial.getLmtData().getNRows());
            vector<unsigned> musclesToUpdate;
            subject.getMusclesIndexFromMusclesList(musclesToUpdate, subject.getMuscleNames());
            ceinms::EmgInformedObjectiveFunctionSetup setup;
            executionCfg_.getMusclesToAdjust(setup.musclesToAdjust);
            executionCfg_.getMusclesToSynthesise(setup.musclesToSynthesise);
            executionCfg_.getHybridWeightings(setup.weightings.alpha, setup.weightings.beta, setup.weightings.gamma);
            executionCfg_.getDofSet(setup.dofsToTrack);
            if (setup.dofsToTrack.empty())
                setup.dofsToTrack = subject.getDoFNames();
            for (const auto& m : setup.musclesToAdjust) {
                auto col = trial.getEmgData().getColumn(m);
                setup.musclesToAdjustMaxExcitations.push_back(*std::max_element(col.begin(), col.end())); 
            }
            
            for (const auto& m : setup.dofsToTrack) {
                auto col = trial.getTorqueData().getColumn(m);
                setup.dofsToTrackMaxTorque.push_back(*std::max_element(col.begin(), col.end(), [](auto e1, auto e2) {
                    return std::abs(e1) < std::abs(e2);
                }));
             }
            
            setup.weightings.beta = beta;
            setup.weightings.gamma = gamma;
            ceinms::HybridEvaluator::evaluate(subject, trial, setup, musclesToUpdate, results);
            std::vector<double> torqueErrors(results.errors.getColumn("TorqueError"));
            double torqueError = std::accumulate(torqueErrors.begin(), torqueErrors.end(), 0.) / torqueErrors.size();
            std::vector<double> trackingError(results.errors.getColumn("ExcitationTrackingError"));
            double trackError = std::accumulate(trackingError.begin(), trackingError.end(), 0.) / trackingError.size();
            HybridErrors errs;
            errs.beta = beta;
            errs.gamma = gamma;
            errs.torqueError = torqueError;
            errs.trackError = trackError;
            return errs;
        });

        auto runGrid([&](vector<double> betas, vector<double> gammas) {
            std::vector<HybridErrors> values;
            for (auto beta : betas)
                for (auto gamma : gammas) {
                        values.emplace_back(f(beta, gamma, subject, trial));
                 }
            return values;
        });

        auto runGridParallel([&](vector<double> betas, vector<double> gammas) {

            std::vector<std::future<HybridErrors>> futures;
            for (auto beta : betas)
                for (auto gamma : gammas) {
                    auto fut(std::async(std::launch::async, [=]() {
                        return f(beta, gamma, subject, trial); }
                    ));
                    futures.emplace_back(std::move(fut));
                }

            std::vector<HybridErrors> values;
            for (auto& f : futures) {
                f.wait();
                values.emplace_back(f.get());
            }
            return values;
        });


        auto getVec([](double min, double max, double interval) {
            vector<double> v;
            int nsteps = round((max - min) / interval);
            for (int i(0); i < nsteps; ++i) {
                v.push_back(min+i*interval);
            }
            return v;
        });


        auto betas = getVec(1, 2, 1);
        auto gammas = getVec(0, 2, 0.1);
        auto values = runGridParallel(betas, gammas);

        std::sort(values.begin(), values.end(), [](auto e1, auto e2) {
            if ((e1.beta - e2.beta) < 0.00001)
                return e1.gamma < e2.gamma;
            return e1.beta < e2.beta;
        });

        std::ofstream outF("values.csv");
            for (auto& v : values) {
                outF << v.beta << "," << v.gamma << "," << v.torqueError << "," << v.trackError << "\n";
        }


        /*
        auto getVec([](int min, int max, int nSteps) {
            vector<double> v;
            int step = (max - min) / nSteps;
            for (min; min < max; min += step) {
                v.push_back(min);
            }
            return v;
        });
        int bmin = 1, bmax = 31, gmin = 1, gmax = 301;
        std::vector<HybridErrors> values;
        vector<double> betas = getVec(bmin, bmax,5);
        vector<double> gammas = getVec(gmin, gmax,5);
        values = runGrid(betas, gammas);
        double maxTorque = std::max_element(values.begin(), values.end(), [](auto e1, auto e2) {
            return e1.torqueError < e2.torqueError;
        })->torqueError;
        double maxTrack = std::max_element(values.begin(), values.end(), [](auto e1, auto e2) {
            return e1.trackError < e2.trackError;
        })->trackError;
        HybridErrors lowest = *std::min_element(values.begin(), values.end(), [maxTorque, maxTrack](auto e1, auto e2) {
            double v1 = std::pow(e1.torqueError / maxTorque, 2) + std::pow(e1.trackError / maxTrack, 2);
            double v2 = std::pow(e2.torqueError / maxTorque, 2) + std::pow(e2.trackError / maxTrack, 2);
            return v1 < v2;
        });
          
        bool terminate = false;
        while (!terminate) {
            cout << "bmin = " << bmin << "\n"
                << "bmax = " << bmax << "\n"
                << "gmin = " << gmin << "\n"
                << "gmax = " << gmax << "\n";

            bmin = (lowest.beta + bmin) / 2;
            bmax = (lowest.beta + bmax) / 2;
            gmin = (lowest.gamma + gmin) / 2;
            gmax = (lowest.gamma + gmax) / 2;

            betas = getVec(bmin, bmax, 5);
            gammas = getVec(gmin, gmax, 5);
            terminate = (betas.size() == 1) && (gammas.size() == 1);
            values = runGrid(betas, gammas);
            HybridErrors lowest = *std::min_element(values.begin(), values.end(), [maxTorque, maxTrack](auto e1, auto e2) {
                double v1 = std::pow(e1.torqueError / maxTorque, 2) + std::pow(e1.trackError / maxTrack, 2);
                double v2 = std::pow(e2.torqueError / maxTorque, 2) + std::pow(e2.trackError / maxTrack, 2);
                return v1 < v2;
            });

            cout << "lowest\nbeta = " << lowest.beta << "\ngamma = " << lowest.gamma << endl;
            cout << "f = " << std::pow(lowest.torqueError / maxTorque, 2) + std::pow(lowest.trackError / maxTrack, 2) << endl;
        }

        std::cout << "Optimal values:\n"
            "alpha = " << 1 << "\n"
            "beta  = " << lowest.beta << "\n"
            "gamma  = " << lowest.gamma << "\n";
        */
        /*
        class WeightingOptimiserSystem : public ceinms::Optimizers::AbstractOptimizerSystem {
        public:
            WeightingOptimiserSystem(NMSmodelT subject, ceinms::TrialData trial, ceinms::ExecutionXmlReader executionCfg) :
                subject_(subject), trial_(trial), executionCfg_(executionCfg),
                beta_(1), gamma_(1), torqueError_(0), trackError_(0), maxTorqueError_(1), maxTrackError_(1) {
            
                // Evaluate max torque error and track error to normalise objective function 
                gamma_ = 0;
                beta_ = 300;
                evaluate();
                maxTrackError_ = trackError_;
                gamma_ = 300;
                evaluate();
                maxTorqueError_ = torqueError_;

                beta_ = 1;
                gamma_ = 1;
            }

            virtual void evaluate() {
   
                int nMtu = subject_.getNoMuscles();
                int nDofs = subject_.getNoDoF();
                ceinms::Result results(nMtu, nDofs, trial_.getEmgData().getNRows(), trial_.getLmtData().getNRows());
                vector<unsigned> musclesToUpdate;
                subject_.getMusclesIndexFromMusclesList(musclesToUpdate, subject_.getMuscleNames());
                ceinms::EmgInformedObjectiveFunctionSetup setup;
                executionCfg_.getMusclesToAdjust(setup.musclesToAdjust);
                executionCfg_.getMusclesToSynthesise(setup.musclesToSynthesise);
                executionCfg_.getHybridWeightings(setup.weightings.alpha, setup.weightings.beta, setup.weightings.gamma);
                setup.weightings.alpha = 1;
                setup.weightings.beta = beta_;
                setup.weightings.gamma = gamma_;
                ceinms::HybridEvaluator::evaluate(subject_, trial_, setup, musclesToUpdate, results);
                std::vector<double> torqueErrors(results.errors.getColumn("TorqueError"));
                torqueError_ = std::accumulate(torqueErrors.begin(), torqueErrors.end(), 0.) / torqueErrors.size();
                std::vector<double> trackingError(results.errors.getColumn("ExcitationTrackingError"));
                trackError_ = std::accumulate(trackingError.begin(), trackingError.end(), 0.) / trackingError.size();
            }
            virtual void setParameters(const std::vector<double>& x) {
                beta_ = x[0];
                gamma_ = x[1];
            }

            virtual std::vector<double> getParameters() const {

                return vector<double>{beta_, gamma_};
            }

            virtual double getF() const {
               double f = (torqueError_ / maxTorqueError_) + (trackError_ / maxTrackError_);
               cout << "f(" << beta_ << "," << gamma_ << ")=" << f << endl;
               return f;
            }


            virtual std::size_t getNoParameters() const {
                return 2;
            }

            void getUpperLowerBounds(std::vector<double>& upperBounds, std::vector<double>& lowerBounds) const {

                upperBounds.resize(getNoParameters());
                upperBounds[0] = 300;
                upperBounds[1] = 300;

                lowerBounds.resize(getNoParameters());
                lowerBounds[0] = .1;
                lowerBounds[1] = .1;
            }

        private:
            NMSmodelT subject_;
            ceinms::TrialData trial_;
            ceinms::ExecutionXmlReader executionCfg_;
            double beta_, gamma_, trackError_, torqueError_, maxTorqueError_, maxTrackError_;
        };
        
        WeightingOptimiserSystem system(subject, trial, executionCfg_);
        ceinms::SimulatedAnnealingParameters params;
        params.noEpsilon = 2;
        params.NS = 10;
        params.NT = 1;
        params.maxNoEval=500;
        params.rt = 0.05;
        params.T = 10000;
        params.epsilon = 0.01;
        ceinms::Optimizers::SimulatedAnnealing<WeightingOptimiserSystem> opt(system, params);
  
        opt.optimize();
        auto x = opt.getOptimizedParameters();
        std::cout << "Optimal values:\n"
            "alpha = " << 1 << "\n"
            "beta  = " << x[0] << "\n"
            "gamma  = " << x[1] << "\n";
            
        /*
        //using variance
        auto getVariance([](const std::vector<double>& v) {
            double ans(0);
            for (auto& e : v)
                ans += e;
            double avg(ans / v.size());
            double var(0), ep(0);
            for (auto& e : v) {
                var += (e - avg)*(e - avg);
                ep += (e - avg);
            }
            return ((var - ep*ep / v.size()) / (v.size() - 1)); //adjust roundoff error.. apparently
        });


        auto getMax([](const std::vector<double>& v) {
            return *std::max_element(v.begin(), v.end());
        });
        double torqueVar = getMax(trial.getTorqueData().getColumn(0));
        double emgVar = 0;
        for (int i(0); i < trial.getEmgData().getNColumns(); ++i) {
            emgVar += getMax(trial.getEmgData().getColumn(i));
        }
        emgVar *= emgVar;
        torqueVar *= torqueVar;
        cout << "var(torque)=" << torqueVar << endl;
        cout << "var(emg)=" << emgVar<< endl;
        cout << "gammavar = " << torqueVar / emgVar << endl;
      
    //    std::ofstream outF("values.csv");
    //    for (auto& v : values) {
    //        outF << v.beta << "," << v.gamma << "," << v.torqueError << "," << v.trackError << "\n";
    //    }
        */
/*
        auto f([&](double beta, double gamma) {
            setup.weightings.beta = beta;
            setup.weightings.gamma = gamma;
            ceinms::HybridEvaluator::evaluate(subject, trial, setup, musclesToUpdate, results);
            std::vector<double> torqueErrors(results.errors.getColumn("TorqueError"));
            double torqueError = std::accumulate(torqueErrors.begin(), torqueErrors.end(), 0.) / torqueErrors.size();
            std::vector<double> trackingError(results.errors.getColumn("ExcitationTrackingError"));
            double trackError = std::accumulate(trackingError.begin(), trackingError.end(), 0.) / trackingError.size();
            double val =  std::pow(torqueError, 2) + std::pow(trackError, 2);
            cout << "f(" << beta << "," << gamma << ")=" << val << endl;
            return val;
        });
    
    
        auto df([&](double beta, double gamma) {
            const double delta = 0.05;
            double df1 = (f(beta + delta, gamma) - f(beta - delta, gamma)) / (2* delta);
            double df2 = (f(beta, gamma + delta) - f(beta, gamma - delta)) / (2* delta);
            cout << "df(" << beta << "," << gamma << ")/dbeta=" << df1 << endl;
            cout << "df(" << beta << "," << gamma << ")/dgamma=" << df2 << endl;
            return vector<double>{df1, df2};
        });


        auto step([&](double beta, double gamma, double stepSize) {
            
            auto grad(df(beta, gamma));
            double beta1 = beta - stepSize*grad[0];
            double gamma1 = gamma - stepSize*grad[1];
            return vector<double>{beta1, gamma1};
        });






        std::cout << "Optimal values:\n"
            "alpha = " << 1 << "\n"
            "beta  = " << x[0] << "\n"
            "gamma  = " << x[1] << "\n";
         
        executionCfg_.setHybridWeightings(1, x[0], x[1]);
        std::string trialId(ceinmsSetup_.getInputDataFile(false));
        size_t endpos = trialId.find(".xml");
        size_t startpos = trialId.rfind("/") + 1;
        trialId = trialId.substr(startpos, endpos - startpos);
        std::string newExecutionFilename(ceinmsSetup_.getExecutionFile(true));
        std::size_t pos = newExecutionFilename.find(".xml");
        if (pos != std::string::npos) {
            newExecutionFilename.insert(pos, "-optimised-"+trialId);
        }
        executionCfg_.print(newExecutionFilename);
        ceinmsSetup_.setExecutionFile(newExecutionFilename);
        
        std::string newSetupFilename(setupFilename_);
        pos = newSetupFilename.find(".xml");
        if (pos != std::string::npos) {
            newSetupFilename.insert(pos, "-optimised");
        }
        ceinmsSetup_.print(newSetupFilename);
        std::cout << "Running best solution\n";
        ceinms::SimulationManager simulationManager(newSetupFilename);
        return simulationManager.run();
        */
        return true;
    }

    ceinms::CeinmsSetupXmlReader ceinmsSetup_;
    ceinms::InputDataXmlReader dataLocations_;
    ceinms::ExecutionXmlReader executionCfg_;
    ceinms::ContactModelXmlReader contactModelCfg_;
    std::string subjectFile_;
    std::string executionFile_;
    std::string inputData_;
    std::string outputDirectory_;
    std::string emgGeneratorFile_;
    std::string setupFilename_;

};


int main(int argc, char** argv) {

    printHeader();
    printAuthors();

#ifdef LOG
    cout << "Check configuration data...\n";
#endif

    string option = "";
    string setupFileName;
    if (argc < 2) {
        PrintUsage();
        return 0;
    }
    else {
        int i;
        for (i = 1; i <= (argc - 1); i++) {
            option = argv[i];

            // PRINT THE USAGE OPTIONS
            if ((option == "-help") || (option == "-h") || (option == "-Help") || (option == "-H") ||
                (option == "-usage") || (option == "-u") || (option == "-Usage") || (option == "-U")) {
                PrintUsage();
                return 0;
            }
            else if ((option == "-S") || (option == "-Setup")) {
                if (argv[i + 1] == 0) {
                    cout << "No setup file specified!" << endl;
                    PrintUsage();
                    return -1;
                }
                setupFileName = argv[i + 1];
                break;

                // Print a default setup file
            }
            else if ((option == "-PrintSetup") || (option == "-PS")) {
                if (ceinms::CeinmsSetupXmlReader::writeTemplateCeinmsSetupFile("defaultCeinmsSetupFile.xml"))
                {
                    std::cout << "Wrote template setup file to defaultCeinmsSetupFile.xml" << std::endl;
                    return 0;
                }
                else
                {
                    std::cout << "An error occurred while writing template setup file to defaultCeinmsSetupFile.xml" << std::endl;
                    return -1;
                }
            }
            else {
                cout << "Unrecognized option " << option << " on command line... Ignored" << endl;
                PrintUsage();
                return -1;
            }
        }
    }


    HybridOptimisationManager simulation(setupFileName);
    bool flag = simulation.run();

    return flag;
}
