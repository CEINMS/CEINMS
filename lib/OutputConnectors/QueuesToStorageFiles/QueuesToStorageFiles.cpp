#include "QueuesToStorageFiles.h"
#include "InputConnectors.h"

namespace ceinms {
    void QueuesToStorageFiles::addLog(std::string valueName)
    {
        std::string filename = valueName + ".sto";
        std::string outFilename = outputDir_ + filename;

        std::shared_ptr<std::ofstream> file_ptr(new std::ofstream(outFilename.c_str()));
        if (!(file_ptr->is_open()))  {
            std::cout << "ERROR: " + filename + " cannot be opened!\n";
            exit(EXIT_FAILURE);
        }

        outFiles_.insert(std::make_pair(valueName, file_ptr));

    }

    void QueuesToStorageFiles::operator()() {

        for (auto& it : valuesToWrite_) {

            auto q = outputConnectors_.logQueues.find(it);
            if (q == outputConnectors_.logQueues.end()) {
                std::cout << it << " queue was not found\n";
                exit(EXIT_FAILURE);
            }
            else {
                q->second->subscribe();
                addLog(it);
            }
        }

        inputConnectors_.doneWithSubscription.wait();

        outputConnectors_.doneWithExecution.wait();


        for (auto& currentValue : valuesToWrite_) {

            auto currentQueue = outputConnectors_.logQueues.find(currentValue);
            if (currentQueue == outputConnectors_.logQueues.end()) {
                std::cout << currentValue << " queue was not found\n";
                exit(EXIT_FAILURE);
            }

            OutputConnectors::FrameType currentFrame;
            std::vector< OutputConnectors::FrameType > dataToWrite;
            currentFrame = currentQueue->second->pop();
            while (currentFrame.time != outputConnectors_.TimePlaceholderForEndOfData) {
                dataToWrite.push_back(currentFrame);
                currentFrame = currentQueue->second->pop();
            }

            initFile(currentValue, dataToWrite);
            auto q = outFiles_.find(currentValue);
            auto outFile = (q->second);
            for (auto& dataIt : dataToWrite) {
                *outFile << dataIt.time << separator_;
                for (auto& sampleIt : dataIt.data)
                    *outFile << sampleIt << separator_;
                *outFile << std::endl;
            }
        }
    }


    void QueuesToStorageFiles::initFile(const std::string& valueName, const std::vector< OutputConnectors::FrameType >& dataToWrite)
    {
        if (dataToWrite.size() < 1)
        {
            std::cout << "No " << valueName << "data to write!" << std::endl;
            return;
        }
        auto q = outFiles_.find(valueName);
        if (q == outFiles_.end()) {
            std::cout << valueName << " file was not found\n";
            exit(EXIT_FAILURE);
        }

        auto outFile = (q->second);
        *outFile << "CEINMS output" << std::endl;
        *outFile << "datacolumns " << dataToWrite.back().data.size() + 1 << std::endl;
        *outFile << "datarows " << dataToWrite.size() << std::endl;
        *outFile << "endheader" << std::endl;
        *outFile << "time" + separator_;
        if (valueName == "Torques")
            for (auto& it : torqueNames_)
                *outFile << it << separator_;
        else
            for (auto& it : muscleNames_)
                *outFile << it << separator_;
        *outFile << std::endl;
    }
}
