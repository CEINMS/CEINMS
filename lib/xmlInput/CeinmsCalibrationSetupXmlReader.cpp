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

#include "CeinmsCalibrationSetupXmlReader.h"
#include "validation.h"

#include "ceinmsCalibrationSetup-schema.hxx"

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

CeinmsCalibrationSetupXmlReader::CeinmsCalibrationSetupXmlReader(const string& filename)
{

    try {
        std::auto_ptr<CeinmsCalibrationType> ceinmsCalibrationDataPointer(parseAndValidate<CeinmsCalibrationType>(filename, ceinmsCalibrationSetup_schema, sizeof(ceinmsCalibrationSetup_schema)));
        ceinmsCalibrationSetupPointer_ = ceinmsCalibrationDataPointer;
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

void CeinmsCalibrationSetupXmlReader::readXml() {

    try {
        std::string& mySubjectFile(*(ceinmsCalibrationSetupPointer_->subjectFile().begin()));
        std::string& myCalibrationExecutionFile(*(ceinmsCalibrationSetupPointer_->calibrationFile().begin()));
        std::string& myEmgGeneratorFile(*(ceinmsCalibrationSetupPointer_->emgGeneratorFile().begin()));
        std::string& myOutputSubjectFile(*(ceinmsCalibrationSetupPointer_->outputSubjectFile().begin()));

    }
    catch (const xml_schema::exception& e) {
        cout << e << endl;
        exit(EXIT_FAILURE);
    }
}

std::string CeinmsCalibrationSetupXmlReader::getSubjectFile()
{
    std::string fileName(*(ceinmsCalibrationSetupPointer_->subjectFile().begin()));
    if (isAbsolute(fileName.c_str()))
        return fileName;
    else
        return filepath_ + fileName;
}

std::string CeinmsCalibrationSetupXmlReader::getCalibrationFile()
{
    std::string fileName(*(ceinmsCalibrationSetupPointer_->calibrationFile().begin()));
    if (isAbsolute(fileName.c_str()))
        return fileName;
    else
        return filepath_ + fileName;
}

std::string CeinmsCalibrationSetupXmlReader::getEmgGeneratorFile()
{
    std::string fileName(*(ceinmsCalibrationSetupPointer_->emgGeneratorFile().begin()));
    if (isAbsolute(fileName.c_str()))
        return fileName;
    else
        return filepath_ + fileName;
}

std::string CeinmsCalibrationSetupXmlReader::getOutputSubjectFile()
{
    std::string fileName(*(ceinmsCalibrationSetupPointer_->outputSubjectFile().begin()));
    if (isAbsolute(fileName.c_str()))
        return fileName;
    else
        return filepath_ + fileName;
}

bool CeinmsCalibrationSetupXmlReader::writeTemplateCeinmsCalibrationSetupFile(const std::string& templateFile)
{
    std::ofstream templateFileStream(templateFile);
    if (!templateFileStream.is_open())
        return false;
    CeinmsCalibrationType templateCeinmsSetup;
    templateCeinmsSetup.subjectFile().push_back("subjectUncalibrated.xml");
    templateCeinmsSetup.calibrationFile().push_back("calibration.xml");
    templateCeinmsSetup.emgGeneratorFile().push_back("emgGenerator.xml");
    templateCeinmsSetup.outputSubjectFile().push_back("subjectCalibrated.xml");
    ceinmsCalibration(templateFileStream, templateCeinmsSetup);
    templateFileStream.close();
    return true;
}
