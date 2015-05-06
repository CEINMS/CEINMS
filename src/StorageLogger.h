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
 * Author(s): C. Pizzolato                                                    *
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

#ifndef ceinms_StorageLogger_h
#define ceinms_StorageLogger_h

#include <string>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/filesystem.hpp>
#include <stdlib.h>
#include "Logger.h"

namespace ceinms {
    namespace Logger {

        template <typename NMSmodelT>
        class StorageLogger {

        public:
            StorageLogger(NMSmodelT& subject, const std::string& outputDir = "./Output/");
            ~StorageLogger();
            void addLog(LogID logID);
            void log(double time, LogID logID);

        private:
            void logDataVector(const std::vector<double>& data, unsigned int index);
            void initFile(unsigned int index);
            NMSmodelT& subject_;

            double time_;
            std::string outputDir_;
            std::string separator_;
            std::vector<boost::shared_ptr<std::ofstream> > outFiles_;
            std::vector < std::vector<std::vector <double> > > dataToWrite_;
            std::vector<LogID> fileTypes_;
        };


        template <typename NMSmodelT>
        StorageLogger<NMSmodelT>::StorageLogger(NMSmodelT& subject, const std::string& outputDir)
            :subject_(subject), outputDir_(outputDir), separator_("\t"){

            outputDir_ += "/";
            boost::filesystem::path dir(outputDir_);
            if (!boost::filesystem::exists(dir)) {
                if (!boost::filesystem::create_directory(dir)) {
                    std::cout << "Error: Cannot create the output directory " + outputDir_ << std::endl;
                    exit(EXIT_FAILURE);
                }
                std::cout << "Created output directory " + dir.string() << std::endl;
            }
            else std::cout << "Using " + dir.string() + " as output directory\n";
        }


        template <typename NMSmodelT>
        void StorageLogger<NMSmodelT>::addLog(LogID logID) {

            std::string filename;
            switch (logID) {
            case Activations:
                filename = "Activations.sto";
                break;
            case FibreLengths:
                filename = "FiberLengths.sto";
                break;
            case FibreVelocities:
                filename = "FiberVelocities.sto";
                break;
            case MuscleForces:
                filename = "MuscleTendonForces.sto";
                break;
            case Torques:
                filename = "Torques.sto";
                break;
            case Emgs:
                filename = "Emgs.sto";
                break;
            case AdjustedEmgs:
                filename = "AdjustedEmgs.sto";
                break;
            }
            std::string outFilename = outputDir_ + filename;
            boost::shared_ptr<std::ofstream> file_ptr(new std::ofstream(outFilename.c_str()));
            outFiles_.push_back(file_ptr);
            dataToWrite_.push_back(std::vector< std::vector <double> >());
            if (!(outFiles_.back()->is_open())) {
                std::cout << "ERROR: " + filename + " cannot be opened!\n";
                exit(EXIT_FAILURE);
            }

            fileTypes_.push_back(logID);
        }


        template <typename NMSmodelT>
        void StorageLogger<NMSmodelT>::log(double time, LogID logID) {

            time_ = time;
            std::vector<double> data;

            switch (logID) {
            case Activations:
                subject_.getActivations(data);
                break;
            case FibreLengths:
                subject_.getFiberLengths(data);
                break;
            case FibreVelocities:
                subject_.getFiberVelocities(data);
                break;
            case MuscleForces:
                subject_.getMuscleForces(data);
                break;
            case Torques:
                subject_.getTorques(data);
                break;
            case Emgs:
            case AdjustedEmgs:
                subject_.getEmgs(data);
                break;
            }

            unsigned dst = std::distance(fileTypes_.begin(),
                std::find(fileTypes_.begin(), fileTypes_.end(),
                logID));
            logDataVector(data, dst);
        }


        template <typename NMSmodelT>
        void StorageLogger<NMSmodelT>::logDataVector(const std::vector<double>& data, unsigned int index) {

            dataToWrite_.at(index).push_back(data);
            dataToWrite_.at(index).back().insert(dataToWrite_.at(index).back().begin(), time_);
        }


        template <typename NMSmodelT>
        void StorageLogger<NMSmodelT>::initFile(unsigned int index) {

            std::ofstream& outFile = *outFiles_.at(index);
            outFile << "CEINMS output" << std::endl;
            outFile << "datacolumns " << dataToWrite_.at(index).back().size() << std::endl;
            outFile << "datarows " << dataToWrite_.at(index).size() << std::endl;
            outFile << "endheader" << std::endl;

            std::vector<std::string> names;
            if (fileTypes_.at(index) == Torques)
                subject_.getDoFNames(names);
            else
                subject_.getMuscleNames(names);

            outFile << "time" + separator_;
            for (unsigned int i = 0; i < names.size() - 1; ++i)
                outFile << names.at(i) << separator_;
            outFile << names.back() << std::endl;
        }

        template <typename NMSmodelT>
        StorageLogger<NMSmodelT>::~StorageLogger() {

            for (size_t iFile = 0; iFile < outFiles_.size() && iFile < dataToWrite_.size(); ++iFile)
            {

                initFile(iFile);

                for (size_t iSample = 0; iSample < dataToWrite_.at(iFile).size(); ++iSample)
                {
                    std::vector<double>& data = dataToWrite_.at(iFile).at(iSample);
                    for (unsigned i = 0; i < data.size() - 1; ++i)
                        *(outFiles_.at(iFile)) << data.at(i) << separator_;
                    *(outFiles_.at(iFile)) << data.back() << std::endl;
                }
                outFiles_.at(iFile)->close();
            }

        }

    };
}
#endif
