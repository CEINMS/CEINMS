//__________________________________________________________________________
// Author(s): Elena Ceseracciu - August 2014
// email:  elena.ceseracciu@gmail.com
//
// DO NOT REDISTRIBUTE WITHOUT PERMISSION
//__________________________________________________________________________
//


#ifndef CeinmsCalibrationSetupXmlReader_h
#define CeinmsCalibrationSetupXmlReader_h

#include <string>
#include <map>
#include "ceinmsCalibrationSetup.hxx"


class CeinmsCalibrationSetupXmlReader {

public:
    CeinmsCalibrationSetupXmlReader(const std::string& filename);
    std::string getSubjectFile();
    std::string getCalibrationFile();
    std::string getExcitationGeneratorFile();
    std::string getOutputSubjectFile();
    static bool writeTemplateCeinmsCalibrationSetupFile(const std::string& templateFile);

private:
    void readXml();
    std::string filepath_;
    std::auto_ptr<CeinmsCalibrationType> ceinmsCalibrationSetupPointer_;
};

#endif
