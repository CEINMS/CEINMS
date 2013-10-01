//__________________________________________________________________________
// Author: Claudio Pizzolato, September 2013
// email:  claudio.pizzolato@griffithuni.edu.au
//
// DO NOT REDISTRIBUTE WITHOUT PERMISSION
//__________________________________________________________________________
//

#ifndef XmlWrite_h
#define XmlWriter_h
#include <string>
#include "subject.hxx"

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
    std::auto_ptr<SubjectType> subjectPointer_;
};

#include "XmlWriter.cpp"

#endif