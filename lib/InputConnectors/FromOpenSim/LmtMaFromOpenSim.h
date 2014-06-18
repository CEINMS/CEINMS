#ifndef LmtMaFromOpenSim_h
#define LmtMaFromOpenSim_h

#include "LmtMaFromX.h"
#include "Utilities.h"
#include "OpenSim/Common/Storage.h"
#include "ArrayConverter.h"

#include <string>
#include <vector>


class LmtMaFromOpenSim:public LmtMaFromX {
public:
    template <typename NMSModelT>
    LmtMaFromOpenSim(const NMSModelT& subject, const OpenSim::Storage& lmtData, const std::vector<OpenSim::Storage>& maData);
    void operator()();

private:
    OpenSim::Storage lmtData_;
    std::vector<OpenSim::Storage> maData_;
    std::vector<size_t> musclePositionInLmtStorage_;
    std::vector< std::vector<size_t> > musclePositionsInMaStorages_;
};

template <typename NMSModelT>
LmtMaFromOpenSim::LmtMaFromOpenSim(const NMSModelT& subject, const OpenSim::Storage& lmtData, const std::vector<OpenSim::Storage>& maData)
:LmtMaFromX(subject), lmtData_(lmtData), maData_(maData)
{
    // 1. setup the conversion matrixes
   OpenSim::Array<std::string> musclesNamesFromStorageFileOsim = lmtData_.getColumnLabels();
   std::vector<std::string> musclesNamesFromStorageFile;
   ArrayConverter::toStdVector<std::string>(musclesNamesFromStorageFileOsim, musclesNamesFromStorageFile);
   musclePositionInLmtStorage_ =  findMapping(musclesNames_, musclesNamesFromStorageFile);

   musclePositionsInMaStorages_.resize( dofNames_.size() );
   size_t currentDof = 0;
   for (auto& it:maData_)
   {
     OpenSim::Array<std::string> muscleNamesFromStorageFileOsim = it.getColumnLabels();
     std::vector<std::string> muscleNamesFromStorageFile;
     ArrayConverter::toStdVector<std::string>(musclesNamesFromStorageFileOsim, musclesNamesFromStorageFile);
     musclePositionsInMaStorages_.at(currentDof) = findMapping(muscleNamesOnDofs_.at(currentDof), muscleNamesFromStorageFile);
     currentDof++;
   }

  // 2. setup the queue for Moment Arms
  for (int i=0; i < dofNames_.size(); ++i)
    CEINMS::InputConnectors::queueMomentArms.push_back(new CEINMS::Concurrency::Queue< CEINMS::InputConnectors::FrameType >);

}

#endif 
