#ifndef ExternalTorquesFromFile_h
#define ExternalTorquesFromFile_h

#include "ExternalTorquesFromX.h"

#include <string>

class ExternalTorquesFromStorageFile:public ExternalTorquesFromX 
{
public:
  template <typename NMSModelT>
  ExternalTorquesFromStorageFile(const NMSModelT& subject, const std::string& dataDirectory);
  void operator()();

private:
  bool externalTorquesFileExist(const std::string& fileName);
  std::string dataDirectory_;
  std::vector<std::size_t> dofPosInStorage_;
};


template <typename NMSModelT>
ExternalTorquesFromStorageFile::ExternalTorquesFromStorageFile(const NMSModelT& subject, const std::string& dataDirectory)
:ExternalTorquesFromX(subject),dataDirectory_(dataDirectory)
{  }

#endif
