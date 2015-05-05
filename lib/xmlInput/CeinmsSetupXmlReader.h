//__________________________________________________________________________
// Author(s): Elena Ceseracciu - August 2014
// email:  elena.ceseracciu@gmail.com
//
// DO NOT REDISTRIBUTE WITHOUT PERMISSION
//__________________________________________________________________________
//


#ifndef ceinms_SetupXmlReader_h
#define ceinms_SetupXmlReader_h

#include <string>
#include <map>
#include "ceinmsSetup.hxx"

namespace ceinms {
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
        std::auto_ptr<CeinmsSetupXsd::CeinmsType> ceinmsSetupPointer_;
    };
}

#endif
