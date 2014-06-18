#ifndef ExternalTorqueFromStorage_h
#define ExternalTorqueFromStorage_h

#include "ExternalTorquesFromX.h"
#include "InputQueues.h"
#include "Utilities.h"

#include <string>
#include <vector>

class ExternalTorquesFromStorage:public ExternalTorquesFromX {
  public:
    template <typename NMSModelT>
    ExternalTorquesFromStorage(const NMSModelT& subject, const std::string& filename);
    void operator()();

  private:
    bool externalTorqueFileExist(const std::string& fileName);
    std::vector<std::size_t> dofPosInStorage_;
    std::string filename_;
};

template <typename NMSModelT>
ExternalTorquesFromStorage::ExternalTorquesFromStorage(const NMSModelT& subject, const std::string& filename)
:ExternalTorquesFromX(subject), filename_(filename)
{
    CEINMS::InputConnectors::externalTorquesAvailable = externalTorqueFileExist(filename_);
}

#endif
