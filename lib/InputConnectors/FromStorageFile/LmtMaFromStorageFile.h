#ifndef ceinms_LmtMaFromStorageFile_h
#define ceinms_LmtMaFromStorageFile_h

#include "LmtMaFromX.h"
#include "Utilities.h"
#include "DataFromStorageFile.h"
#include <string>
#include <vector>

namespace ceinms {
    class LmtMaFromStorageFile :public LmtMaFromX {
    public:
        template <typename NMSModelT>
        LmtMaFromStorageFile(InputConnectors& inputConnectors, const NMSModelT& subject, const std::string& lmtDataFilename, const std::vector< std::string>& maDataFileName);
        void operator()();

    private:
        std::string dataDirectory_;
        std::vector<size_t> musclePositionInLmtStorage_;
        std::vector< std::vector<size_t> > musclePositionsInMaStorages_;
        DataFromStorageFile lmtData_;
        std::vector< DataFromStorageFile* > maDataStorageFiles_;
    };

    template <typename NMSModelT>
    LmtMaFromStorageFile::LmtMaFromStorageFile(InputConnectors& inputConnectors, const NMSModelT& subject, const std::string& lmtDataFilename, const std::vector< std::string>& maDataFileName)
        :LmtMaFromX(inputConnectors, subject), lmtData_(lmtDataFilename)
    {  // 1. Open the input files
        for (std::string it : maDataFileName)
        {
            DataFromStorageFile* nextMaDataPtr = new DataFromStorageFile(it);
            maDataStorageFiles_.push_back(nextMaDataPtr);
        }

        // 2. setup the conversion matrixes
        std::vector<std::string> musclesNamesFromStorageFile = lmtData_.getColumnNames();
        musclePositionInLmtStorage_ = findMapping(musclesNames_, musclesNamesFromStorageFile);

        musclePositionsInMaStorages_.resize(dofNames_.size());
        size_t currentDof = 0;
        for (auto& it : maDataStorageFiles_)
        {
            std::vector<std::string> muscleNamesFromStorageFile = it->getColumnNames();
            musclePositionsInMaStorages_.at(currentDof) = findMapping(muscleNamesOnDofs_.at(currentDof), muscleNamesFromStorageFile);
            currentDof++;
        }

        inputConnectors_.queueMomentArms.clear();
        for (int i = 0; i < dofNames_.size(); ++i)
            inputConnectors_.queueMomentArms.push_back(new Concurrency::Queue< InputConnectors::FrameType >);

    }
}

#endif
