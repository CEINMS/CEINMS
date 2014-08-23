//__________________________________________________________________________
// Author(s): Elena Ceseracciu - August 2014
// email:  elena.ceseracciu@gmail.com
//
// DO NOT REDISTRIBUTE WITHOUT PERMISSION
//__________________________________________________________________________
//

#include <string>
using std::string;
#include <map>
using std::map;
#include <iostream>
using std::cout;
using std::endl;
#include <fstream>
#include "ceinmsSetup-schema.hxx"
#include "validation.h"

#include "CeinmsSetupXmlReader.h"

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

CeinmsSetupXmlReader::CeinmsSetupXmlReader(const string& filename)
{

    try {
        std::auto_ptr<CeinmsType> ceinmsDataPointer(parseAndValidate<CeinmsType>(filename, ceinmsSetup_schema, sizeof(ceinmsSetup_schema)));
        ceinmsSetupPointer_ = ceinmsDataPointer;
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

void CeinmsSetupXmlReader::readXml() {

    try {
        std::string& mySubjectFile(*(ceinmsSetupPointer_->subjectFile().begin()));
        std::string& myInputDataFile(*(ceinmsSetupPointer_->inputDataFile().begin()));
        std::string& myExecutionFile(*(ceinmsSetupPointer_->executionFile().begin()));
        std::string& myEmgGeneratorFile(*(ceinmsSetupPointer_->emgGeneratorFile().begin()));
        std::string& myOutputDirectory(*(ceinmsSetupPointer_->outputDirectory().begin()));

    }
    catch (const xml_schema::exception& e) {
        cout << e << endl;
        exit(EXIT_FAILURE);
    }
}

std::string CeinmsSetupXmlReader::getSubjectFile()
{
    std::string fileName(*(ceinmsSetupPointer_->subjectFile().begin()));
     if (isAbsolute(fileName.c_str()))
            return fileName;
        else
            return filepath_ + fileName;
}

std::string CeinmsSetupXmlReader::getInputDataFile()
{
    std::string fileName(*(ceinmsSetupPointer_->inputDataFile().begin()));
    if (isAbsolute(fileName.c_str()))
        return fileName;
    else
        return filepath_ + fileName;
}

std::string CeinmsSetupXmlReader::getExecutionFile()
{
    std::string fileName(*(ceinmsSetupPointer_->executionFile().begin()));
    if (isAbsolute(fileName.c_str()))
        return fileName;
    else
        return filepath_ + fileName;
}

std::string CeinmsSetupXmlReader::getEmgGeneratorFile()
{
    std::string fileName(*(ceinmsSetupPointer_->emgGeneratorFile().begin()));
    if (isAbsolute(fileName.c_str()))
        return fileName;
    else
        return filepath_ + fileName;
}

std::string CeinmsSetupXmlReader::getOutputDirectory()
{
    std::string fileName(*(ceinmsSetupPointer_->outputDirectory().begin()));
    if (isAbsolute(fileName.c_str()))
        return fileName;
    else
        return filepath_ + fileName;
}

bool CeinmsSetupXmlReader::writeTemplateCeinmsSetupFile(const string& templateFile)
{
    std::ofstream templateFileStream(templateFile);
    if (!templateFileStream.is_open())
        return false;
    CeinmsType templateCeinmsSetup;
    templateCeinmsSetup.subjectFile().push_back("subject.xml");
    templateCeinmsSetup.inputDataFile().push_back("inputData.xml");
    templateCeinmsSetup.executionFile().push_back("execution.xml");
    templateCeinmsSetup.emgGeneratorFile().push_back("emgGenerator.xml");
    templateCeinmsSetup.outputDirectory().push_back("./Output");
    ceinms (templateFileStream, templateCeinmsSetup);
    templateFileStream.close();
    return true;
}
