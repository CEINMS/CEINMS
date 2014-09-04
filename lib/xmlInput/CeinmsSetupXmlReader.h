//__________________________________________________________________________
// Author(s): Elena Ceseracciu - August 2014
// email:  elena.ceseracciu@gmail.com
//
// DO NOT REDISTRIBUTE WITHOUT PERMISSION
//__________________________________________________________________________
//


#ifndef CeinmsSetupXmlReader_h
#define CeinmsSetupXmlReader_h

#include <string>
#include <map>
#include "ceinmsSetup.hxx"


class CeinmsSetupXmlReader {

public:
    CeinmsSetupXmlReader(const std::string& filename);
    std::string getSubjectFile();
    std::string getInputDataFile();
    std::string getExecutionFile();
    std::string getExcitationGeneratorFile();
    std::string getOutputDirectory();
    static bool writeTemplateCeinmsSetupFile(const std::string& templateFile);

private:
    void readXml();
    std::string filepath_;
    std::auto_ptr<CeinmsType> ceinmsSetupPointer_;
};

#endif
