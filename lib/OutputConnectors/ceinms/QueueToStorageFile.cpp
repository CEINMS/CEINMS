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
* Author(s): E. Ceseracciu, C. Pizzolato, M. Reggiani                        *
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
//#include "ceinms/QueueToStorageFile.h"
#include "ceinms/QueuesSync.h"
#include <iostream>
#include <vector>
#include <SimTK/include/SimTKcommon.h>
#include "ceinms/GeneralisedCoordinatesData.h"
#include "ceinms/ExternalForceData.h"
#include "ceinms/queues/MultipleExternalForcesQueue.h"

namespace ceinms{

    template<typename DataType>
    QueueToStorageFile<DataType>::QueueToStorageFile(
        QueueType& inputQueue,
        Concurrency::Latch& subscriptionLatch,
        Concurrency::Latch& readyToWriteLatch,
        const std::vector<std::string>& columnLabels,
        const std::string& valueName,
        const std::string& outputDir,
        const std::string& extension) :
        inputQueue_(inputQueue),
        subscriptionLatch_(subscriptionLatch),
        readyToWriteLatch_(readyToWriteLatch),
        columnLabels_(columnLabels),
        valueName_(valueName),
        outputDir_(outputDir),
        separator_("\t"),
        extension_(extension)
    {

        outputDir_ += "/";
        boost::filesystem::path dir(outputDir_);
        if (!boost::filesystem::exists(dir)) {
            if (!boost::filesystem::create_directories(dir)) {
                std::cout << "Error: Cannot create the output directory " + outputDir_ << std::endl;
                exit(EXIT_FAILURE);
            }
            std::cout << "Created output directory " + dir.string() << std::endl;
        }
        else std::cout << "Using " + dir.string() + " as output directory\n";

        std::string filename = valueName + "." + extension_;
        std::string outFilename = outputDir_ + filename;

        outFile_.reset(new std::ofstream(outFilename.c_str()));

        if (!(outFile_->is_open()))  {
            std::cout << "ERROR: " + filename + " cannot be opened!\n";
            exit(EXIT_FAILURE);
        }
    }

    template<typename DataType>
    void QueueToStorageFile<DataType>::initFile()
    {
        if (dataToWrite_.size() < 1)
        {
            std::cout << "No " << valueName_ << "data to write!" << std::endl;
            return;
        }

        *outFile_ << "CEINMS output" << std::endl;
        *outFile_ << "datacolumns " << dataToWrite_.back().data.size() + 1 << std::endl;
        *outFile_ << "datarows " << dataToWrite_.size() << std::endl;
        *outFile_ << "endheader" << std::endl;
        *outFile_ << "time" + separator_;
        for (auto& it : columnLabels_)
            *outFile_ << it << separator_;
        *outFile_ << std::endl;
    }

    template<>
    void QueueToStorageFile<std::vector<SimTK::Vec3>>::initFile()
    {
        if (dataToWrite_.size() < 1)
        {
            std::cout << "No " << valueName_ << "data to write!" << std::endl;
            return;
        }

        *outFile_ << "CEINMS output" << std::endl;
        *outFile_ << "datacolumns " << dataToWrite_.back().data.size() * 3 + 1 << std::endl;
        *outFile_ << "datarows " << dataToWrite_.size() << std::endl;
        *outFile_ << "endheader" << std::endl;
        *outFile_ << "time" + separator_;
        for (auto& it : columnLabels_)
            *outFile_ << it << "_x" << separator_ << it << "_y" << separator_ << it << "_z" << separator_;
        *outFile_ << std::endl;
    }

    template<>
    void QueueToStorageFile<GeneralisedCoordinatesData>::initFile()
    {
        if (dataToWrite_.size() < 1)
        {
            std::cout << "No " << valueName_ << "data to write!" << std::endl;
            return;
        }

        *outFile_ << "CEINMS output" << std::endl;
        *outFile_ << "datacolumns " << dataToWrite_.back().data.getNCoordinates() * 3 + 1 << std::endl;
        *outFile_ << "datarows " << dataToWrite_.size() << std::endl;
        *outFile_ << "endheader" << std::endl;
        *outFile_ << "time" + separator_;
        for (auto& it : columnLabels_)
            *outFile_ << it << "_pos" << separator_ << it << "_vel" << separator_ << it << "_acc" << separator_;
        *outFile_ << std::endl;
    }

    template<>
    void QueueToStorageFile<MultipleExternalForcesData>::initFile()
    {
        if (dataToWrite_.size() < 1)
        {
            std::cout << "No " << valueName_ << "data to write!" << std::endl;
            return;
        }

        *outFile_ << "CEINMS output" << std::endl;
        *outFile_ << "datacolumns " << dataToWrite_.back().data.size() * 9 + 1 << std::endl;
        *outFile_ << "datarows " << dataToWrite_.size() << std::endl;
        *outFile_ << "endheader" << std::endl;
        *outFile_ << "time" + separator_;
        for (auto& it : columnLabels_)
            *outFile_ <<
            it << "_vx" << separator_ <<
            it << "_vy" << separator_ <<
            it << "_vz" << separator_ <<
            it << "_px" << separator_ <<
            it << "_py" << separator_ <<
            it << "_pz" << separator_ <<
            it << "_mx" << separator_ <<
            it << "_my" << separator_ <<
            it << "_mz" << separator_;
        *outFile_ << std::endl;
    }

    template<typename DataType>
    void QueueToStorageFile<DataType>::operator()() {

        inputQueue_.subscribe();
        subscriptionLatch_.wait();
        readyToWriteLatch_.wait();

        bool runCondition(true);
        while (runCondition) {
            FrameType currentFrame = inputQueue_.pop();
            if (QueuesSync::isEndOfData(currentFrame)) runCondition = false;
            if (runCondition) {
                dataToWrite_.push_back(currentFrame);
            }
        }

        initFile();
        writeToFile();
    }

    template<typename DataType>
    void QueueToStorageFile<DataType>::writeToFile() {

        for (auto& dataIt : dataToWrite_) {
            *outFile_ << dataIt.time << separator_;
            for (auto& sampleIt : dataIt.data)
                *outFile_ << sampleIt << separator_;
            *outFile_ << std::endl;
        }
    }

    template<>
    void QueueToStorageFile<std::vector<SimTK::Vec3>>::writeToFile() {

        for (auto& dataIt : dataToWrite_) {
            *outFile_ << dataIt.time << separator_;
            for (auto& sampleIt : dataIt.data)
                *outFile_ << sampleIt[0] << separator_
                << sampleIt[1] << separator_
                << sampleIt[2] << separator_;
            *outFile_ << std::endl;
        }
    }

    template<>
    void QueueToStorageFile<GeneralisedCoordinatesData>::writeToFile() {

        for (auto& dataIt : dataToWrite_) {
            *outFile_ << dataIt.time << separator_;
            auto q(dataIt.data.getQ());
            auto qd(dataIt.data.getQd());
            auto qdd(dataIt.data.getQdd());

            for (unsigned i(0); i < dataIt.data.getNCoordinates(); ++i) {
                *outFile_ << q[i] << separator_
                    << qd[i] << separator_
                    << qdd[i] << separator_;
            }
            *outFile_ << std::endl;
        }
    }

    template<>
    void QueueToStorageFile<MultipleExternalForcesData>::writeToFile() {

        for (auto& dataIt : dataToWrite_) {
            *outFile_ << dataIt.time << separator_;
            for (auto& v : dataIt.data) {
                auto f(v.getForce());
                auto p(v.getApplicationPoint());
                auto m(v.getTorque());
                *outFile_
                    << f[0] << separator_
                    << f[1] << separator_
                    << f[2] << separator_
                    << p[0] << separator_
                    << p[1] << separator_
                    << p[2] << separator_
                    << m[0] << separator_
                    << m[1] << separator_
                    << m[2] << separator_;
            }
            *outFile_ << std::endl;
        }
    }
}