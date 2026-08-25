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

#ifndef ceinms_FileLogger_h
#define ceinms_FileLogger_h

#include <filesystem>
#include <vector>
#include <list>
#include <memory>
#include <string>
#include <fstream>
#include <iostream>

#include "ceinms/ExternalForceData.h"
#include "ceinms/queues/MultipleExternalForcesQueue.h"
#include "ceinms/GeneralisedCoordinatesData.h"

#include <SimTKcommon.h>

namespace ceinms {

    template<typename DataType>
    class FileLogger {
    public:
        using FrameType = QueueData<DataType> ;
        using QueueType = Concurrency::Queue<FrameType> ;
        FileLogger(
            const std::vector<std::string>& columnLabels,
            const std::string& filename,
            const std::string& outputDir = "./Output/",
            const std::string& separator = "\t"
        );
        void logFrame(const FrameType& frame);
        void setFilename(const std::string& filename);
        void setOutputDir(const std::string& outputDir);
        void setColumnLabels(const std::vector<std::string>& columnLabels);
        std::string getFilename() { return filename_; }
        void clearData();
        bool print();
        bool printable();

    private:
        bool createFile();
        void initFile();
        void writeToFile();
        std::vector<std::string> columnLabels_;
        std::string outputDir_;
        std::string sp_;
        std::string filename_;
        std::shared_ptr<std::ofstream> outFile_;
        std::list< FrameType > dataToWrite_;
    };

    template<typename DataType>
    FileLogger<DataType>::FileLogger(
        const std::vector<std::string>& columnLabels,
        const std::string& filename,
        const std::string& outputDir,
        const std::string& separator
        ) :
        columnLabels_(columnLabels),
        filename_(filename),
        outputDir_(outputDir),
        sp_(separator)
    {    }

    template<typename DataType>
    void FileLogger<DataType>::logFrame(const FrameType& frame) {
        dataToWrite_.push_back(frame);
    }

    template<typename DataType>
    void FileLogger<DataType>::setFilename(const std::string& filename) {
        filename_ = filename;
    }

    template<typename DataType>
    void FileLogger<DataType>::setOutputDir(const std::string& outputDir) {
        outputDir_ = outputDir;
    }

    template<typename DataType>
    void FileLogger<DataType>::setColumnLabels(const std::vector<std::string>& columnLabels) {
        columnLabels_ = columnLabels;
    }

    template<typename DataType>
    void FileLogger<DataType>::clearData() {
        dataToWrite_.clear();
    }

    //return true if printed successfully
    template<typename DataType>
    bool FileLogger<DataType>::print() {
        if (dataToWrite_.size() < 1) {
            std::cout << "No data to write in " << filename_ << std::endl;
            return false;
        }
        if (!createFile())
            return false;
        initFile();
        writeToFile();
        outFile_->flush();
        outFile_->close();
        return true;
    }

    template<typename DataType>
    bool FileLogger<DataType>::printable() {
        if (createFile()) {
            outFile_->close();
            return true;
        }
        return false;
    }

    template<typename DataType>
    bool FileLogger<DataType>::createFile() {
        bool success(true);
        std::filesystem::path dir(outputDir_);
        if (!std::filesystem::exists(dir)) {
            // create_directories() reports whether it created the final path component.
            // outputDir_ ends in a separator, so that component is empty and it returns false
            // even when it did create the directory. The run then aborted having already made
            // the directory, which is why running again worked. Check for the directory instead
            // of trusting the return value.
            std::error_code ec;
            std::filesystem::create_directories(dir, ec);
            if (!std::filesystem::exists(dir)) {
                std::cout << "Error: Cannot create the output directory " + outputDir_
                    << ": " << ec.message() << std::endl;
                success = false;
            }
            else std::cout << "Created output directory " + dir.string() << std::endl;
        }
        else std::cout << "Using " + std::filesystem::canonical(dir).string() + " as output directory\n";

        std::string outFilename = (dir / filename_).string();

        outFile_.reset(new std::ofstream(outFilename.c_str()));

        if (!(outFile_->is_open())) {
            std::cout << "ERROR: " + filename_ + " cannot be opened!\n";
            success = false;
        }
        return success;
    }

    template<typename DataType>
    void FileLogger<DataType>::initFile() {
        *outFile_ << "CEINMS output" << std::endl;
        *outFile_ << "datacolumns " << dataToWrite_.back().data.size() + 1 << std::endl;
        *outFile_ << "datarows " << dataToWrite_.size() << std::endl;
        *outFile_ << "endheader" << std::endl;
        *outFile_ << "time" + sp_;
        for (auto& it : columnLabels_)
            *outFile_ << it << sp_;
        *outFile_ << std::endl;
    }

    template<>
    void FileLogger<std::vector<SimTK::Vec3>>::initFile() {
        *outFile_ << "CEINMS output" << std::endl;
        *outFile_ << "datacolumns " << dataToWrite_.back().data.size() * 3 + 1 << std::endl;
        *outFile_ << "datarows " << dataToWrite_.size() << std::endl;
        *outFile_ << "endheader" << std::endl;
        *outFile_ << "time" + sp_;
        for (auto& it : columnLabels_)
            *outFile_ << it << "_x" << sp_ << it << "_y" << sp_ << it << "_z" << sp_;
        *outFile_ << std::endl;
    }

    template<>
    void FileLogger<GeneralisedCoordinatesData>::initFile() {
        *outFile_ << "CEINMS output" << std::endl;
        *outFile_ << "datacolumns " << dataToWrite_.back().data.getNCoordinates() * 3 + 1 << std::endl;
        *outFile_ << "datarows " << dataToWrite_.size() << std::endl;
        *outFile_ << "endheader" << std::endl;
        *outFile_ << "time" + sp_;
        for (auto& it : columnLabels_)
            *outFile_ << it << sp_ << it << "_vel" << sp_ << it << "_acc" << sp_;
        *outFile_ << std::endl;
    }

    template<>
    void FileLogger<MultipleExternalForcesData>::initFile() {
        *outFile_ << "CEINMS output" << std::endl;
        *outFile_ << "datacolumns " << dataToWrite_.back().data.size() * 12 + 1 << std::endl;
        *outFile_ << "datarows " << dataToWrite_.size() << std::endl;
        *outFile_ << "endheader" << std::endl;
        *outFile_ << "time" + sp_;
        for (auto& it : columnLabels_)
            *outFile_ <<
            it << "_vx" << sp_ <<
            it << "_vy" << sp_ <<
            it << "_vz" << sp_ <<
            it << "_px" << sp_ <<
            it << "_py" << sp_ <<
            it << "_pz" << sp_ <<
            it << "_mx" << sp_ <<
            it << "_my" << sp_ <<
            it << "_mz" << sp_ <<
            it << "_tx" << sp_ <<
            it << "_ty" << sp_ <<
            it << "_tz" << sp_;
        ;
        *outFile_ << std::endl;
    }

    template<typename DataType>
    void FileLogger<DataType>::writeToFile() {
        for (auto& dataIt : dataToWrite_) {
            *outFile_ << dataIt.time << sp_;
            for (auto& sampleIt : dataIt.data)
                *outFile_ << sampleIt << sp_;
            *outFile_ << std::endl;
        }
    }

    template<>
    void FileLogger<std::vector<SimTK::Vec3>>::writeToFile() {
        for (auto& dataIt : dataToWrite_) {
            *outFile_ << dataIt.time << sp_;
            for (auto& sampleIt : dataIt.data)
                *outFile_ << sampleIt[0] << sp_
                << sampleIt[1] << sp_
                << sampleIt[2] << sp_;
            *outFile_ << std::endl;
        }
    }

    template<>
    void FileLogger<GeneralisedCoordinatesData>::writeToFile() {
        for (auto& dataIt : dataToWrite_) {
            *outFile_ << dataIt.time << sp_;
            auto q(dataIt.data.getQ());
            auto qd(dataIt.data.getQd());
            auto qdd(dataIt.data.getQdd());

            for (unsigned i(0); i < dataIt.data.getNCoordinates(); ++i) {
                *outFile_ << q[i] << sp_
                    << qd[i] << sp_
                    << qdd[i] << sp_;
            }
            *outFile_ << std::endl;
        }
    }

    template<>
    void FileLogger<MultipleExternalForcesData>::writeToFile() {
        for (auto& dataIt : dataToWrite_) {
            *outFile_ << dataIt.time << sp_;
            for (auto& v : dataIt.data) {
                auto f(v.getForce());
                auto p(v.getApplicationPoint());
                auto m(v.getMoments());
                auto t(v.getTorque());
                *outFile_
                    << f[0] << sp_
                    << f[1] << sp_
                    << f[2] << sp_
                    << p[0] << sp_
                    << p[1] << sp_
                    << p[2] << sp_
                    << m[0] << sp_
                    << m[1] << sp_
                    << m[2] << sp_
                    << t[0] << sp_
                    << t[1] << sp_
                    << t[2] << sp_;
            }
            *outFile_ << std::endl;
        }
    }
}

#endif
