//__________________________________________________________________________
// Author(s): Claudio Pizzolato - October 2013
// email:  claudio.pizzolato@griffithuni.edu.au
//
// DO NOT REDISTRIBUTE WITHOUT PERMISSION
//__________________________________________________________________________
//

#ifndef ceinms_XmlWrite_h
#define ceinms_XmlWriter_h
#include <string>
#include "subject.hxx"

namespace ceinms {
    class XmlWriter {

    public:
        // XmlWriter();
        XmlWriter(const std::string& oldXmlFile);
        void setOutputFilename(const std::string& outFilename);

        template<typename NMSmodelT>
        void setParameters(NMSmodelT& subject);
        void addCalibrationInfo(const std::string& calibrationXmlFilename);
        void write();

    private:
        std::string outFilename_;
        std::string startXmlFile_;
        //   SubjectType s_;
        std::auto_ptr<SubjectXsd::SubjectType> subjectPointer_;
    };
}
#include "XmlWriter.cpp"

#endif