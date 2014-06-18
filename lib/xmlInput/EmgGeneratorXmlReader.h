//__________________________________________________________________________
// Author(s): Elena Ceseracciu - June 2014
// email:  elena.ceseracciu@gmail.com
//
// DO NOT REDISTRIBUTE WITHOUT PERMISSION
//__________________________________________________________________________
//

#ifndef SubjectXmlReader_h
#define SubjectXmlReader_h

#include "emgGenerator.hxx"
#include <string>


class EmgGeneratorXmlReader {

public:
    EmgGeneratorXmlReader(const std::string& filename);
    std::auto_ptr<emgGenerator> getEmgGenerator();
    friend std::ostream& operator<< (std::ostream& output, const EmgGeneratorXmlReader& b);
private:
    std::auto_ptr<emgGenerator> emgGenPointer_;
};

#endif