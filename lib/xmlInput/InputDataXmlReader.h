//__________________________________________________________________________
// Author(s): Elena Ceseracciu - August 2014
// email:  elena.ceseracciu@gmail.com
//
// DO NOT REDISTRIBUTE WITHOUT PERMISSION
//__________________________________________________________________________
//


#ifndef InputDataXmlReader_h
#define InputDataXmlReader_h

#include <string>
#include <map>
#include "inputData.hxx"


class InputDataXmlReader {

public:
    InputDataXmlReader(const std::string& filename);
    std::string getLmtFile();
    std::map<std::string, std::string> getMaFiles();
    std::string getEmgFile();
    std::string getExternalTorqueFile();

private:
    void readXml();
    std::string filepath_;
    std::auto_ptr<InputDataType> inputDataPointer_;
};

#endif