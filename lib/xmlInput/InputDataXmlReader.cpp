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
    //TODO: check that the provided location is not absolute
    return filepath_ + *(inputDataPointer_->muscleTendonLengthFile().begin());
}

std::map<std::string, std::string> InputDataXmlReader::getMaFiles()
{
    map<string, string> result;
    for (auto it : inputDataPointer_->momentArmsFiles().begin()->momentArmsFile())
    {
        std::string pathToMaFile= filepath_+ it; //TODO: check that the provided location is not absolute
        result.insert(std::pair<string, string>(it.dofName(), pathToMaFile));
    }
    // return filepath_ + *(inputDataPointer_->momentArmsDirectory().begin());
    return result;
}

std::string InputDataXmlReader::getEmgFile()
{
    //TODO: check that the provided location is not absolute
    return filepath_ + *(inputDataPointer_->emgFile().begin());
}

std::string InputDataXmlReader::getExternalTorqueFile()
{
    //TODO: check that the provided location is not absolute
    return filepath_ + *(inputDataPointer_->externalTorquesFile().begin());
}

