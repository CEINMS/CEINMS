#ifndef ceinms_EMGFromFile_h
#define ceinms_EMGFromFile_h

#include <string>
#include <vector>
#include <iostream>

#include "EMGFromX.h"
#include "EMGDataFromFile.h"
#include "ExcitationGeneratorFromXml.h"
#include "InputConnectors.h"

#define LOG

namespace ceinms {
    class EMGFromFile :public EMGFromX
    {
    public:
        template <typename NMSModelT>
        EMGFromFile(InputConnectors& inputConnectors, const NMSModelT& subject, const std::string& emgFilename, const std::string& emgGeneratorFilename);
        void operator()();
        virtual ~EMGFromFile();
    private:
        std::string inputDir_;
        EMGDataFromFile<ExcitationGeneratorFromXml> emgData_;
    };


    template <typename NMSModelT>
    EMGFromFile::EMGFromFile(InputConnectors& inputConnectors, const NMSModelT& subject, const std::string& emgFilename, const std::string& emgGeneratorFilename)
        :EMGFromX(inputConnectors, subject), emgData_(emgFilename, emgGeneratorFilename)
    {

#ifdef LOG  
        std::cout << "\nEMG: Reading emg file..." << emgFilename << std::endl;
#endif

        std::vector<std::string> emgMusclesNames;
        emgData_.getMusclesNames(emgMusclesNames);
        if (!(musclesNames_ == emgMusclesNames)) {
            std::cout << "\n\n ERROR: Muscle names from emg file and subject differ!\n";
            std::cout << "Subject muscles: ";
            for (auto it : musclesNames_)
                std::cout << it << "\t";
            std::cout << "\nEMG muscles: ";
            for (auto it : emgMusclesNames)
                std::cout << it << "\t";
            std::cout << std::endl;
            exit(EXIT_FAILURE);
        }
    }
}

#endif
