//__________________________________________________________________________
// Author(s): Elena Ceseracciu - August 2014
// email:  elena.ceseracciu@gmail.com
//
// DO NOT REDISTRIBUTE WITHOUT PERMISSION
//__________________________________________________________________________
//


#include "InputDataXmlReader.h"
#include <string>
using std::string;
#include <map>
using std::map;
#include <iostream>
using std::cout;
using std::endl;
#include "inputData-schema.hxx"
#include "validation.h"

inline bool isAbsolute(const char *path) {
    if (path[0] == '/' || path[0] == '\\') {
        return true;
    }
    std::string str(path);
    if (str.length()>1) {
        if (str[1] == ':') {
            return true;
        }
    }
    return false;
};

InputDataXmlReader::InputDataXmlReader(const string& filename)
{

    try {
        std::auto_ptr<InputDataType> inputDataPointer(parseAndValidate<InputDataType>(filename, inputData_schema, sizeof(inputData_schema)));
        inputDataPointer_ = inputDataPointer;
    }
    catch (const xml_schema::exception& e) {
        cout << e << endl;
        exit(EXIT_FAILURE);
    }

    // save the location of input filename
    string::size_type dirSep = filename.rfind('/'); // Unix/Mac dir separator

    if (dirSep == string::npos)
        dirSep = filename.rfind('\\'); // DOS dir separator

    if (dirSep != string::npos) // if '_fileName' contains path information...
        filepath_ = filename.substr(0, dirSep + 1); // include trailing slashes

    readXml();
}

void InputDataXmlReader::readXml() {

    try {
        std::string& myLmtFile(*(inputDataPointer_->muscleTendonLengthFile().begin()));
        std::string& myEmgFile(*(inputDataPointer_->emgFile().begin()));
       //TODO std::string& myMomentArmDir(*(inputDataPointer_->momentArmsFDirectory().begin()));
        std::string& myExtTorqueFile(*(inputDataPointer_->externalTorquesFile().begin()));
    }
    catch (const xml_schema::exception& e) {
        cout << e << endl;
        exit(EXIT_FAILURE);
    }
}

std::string InputDataXmlReader::getLmtFile()
{
    std::string fileName(*(inputDataPointer_->muscleTendonLengthFile().begin()));
     if (isAbsolute(fileName.c_str()))
            return fileName;
        else
            return filepath_ + fileName;
}

std::map<std::string, std::string> InputDataXmlReader::getMaFiles()
{
    map<string, string> result;
    for (auto it : inputDataPointer_->momentArmsFiles().begin()->momentArmsFile())
    {
        std::string pathToMaFile;
        if (isAbsolute(it.c_str()))
            pathToMaFile = it;
        else
            pathToMaFile = filepath_ + it;
        result.insert(std::pair<string, string>(it.dofName(), pathToMaFile));
    }
    // return filepath_ + *(inputDataPointer_->momentArmsDirectory().begin());
    return result;
}

std::string InputDataXmlReader::getEmgFile()
{
    std::string fileName(*(inputDataPointer_->emgFile().begin()));
    if (isAbsolute(fileName.c_str()))
        return fileName;
    else
        return filepath_ + fileName;
}

std::string InputDataXmlReader::getExternalTorqueFile()
{
    std::string fileName(*(inputDataPointer_->externalTorquesFile().begin()));
    if (isAbsolute(fileName.c_str()))
        return fileName;
    else
        return filepath_ + fileName;
}

