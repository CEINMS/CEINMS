//__________________________________________________________________________
// Author(s): Elena Ceseracciu - August 2014
// email:  elena.ceseracciu@gmail.com
//
// DO NOT REDISTRIBUTE WITHOUT PERMISSION
//__________________________________________________________________________
//


#ifndef ceinms_InputDataXmlReader_h
#define ceinms_InputDataXmlReader_h

#include <string>
#include <map>
#include "inputData.hxx"

namespace ceinms {
    class InputDataXmlReader {
    public:
        InputDataXmlReader(const std::string& filename);
        std::string getLmtFile();
        std::map<std::string, std::string> getMaFiles();
        std::string getExcitationsFile();
        std::string getExternalTorqueFile();

    private:
        void readXml();
        std::string filepath_;
        std::auto_ptr<InputDataXsd::InputDataType> inputDataPointer_;
    };
}

#endif