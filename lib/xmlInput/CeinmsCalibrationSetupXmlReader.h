//__________________________________________________________________________
// Author(s): Elena Ceseracciu - August 2014
// email:  elena.ceseracciu@gmail.com
//
// DO NOT REDISTRIBUTE WITHOUT PERMISSION
//__________________________________________________________________________
//


#ifndef ceinms_CalibrationSetupXmlReader_h
#define ceinms_CalibrationSetupXmlReader_h

#include <string>
#include <map>
#include "ceinmsCalibrationSetup.hxx"

namespace ceinms {
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
        std::auto_ptr<CeinmsCalibrationSetupXsd::CeinmsCalibrationType> ceinmsCalibrationSetupPointer_;
    };
}

#endif
