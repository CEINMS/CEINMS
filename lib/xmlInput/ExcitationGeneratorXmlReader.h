//__________________________________________________________________________
// Author(s): Elena Ceseracciu - June 2014
// email:  elena.ceseracciu@gmail.com
//
// DO NOT REDISTRIBUTE WITHOUT PERMISSION
//__________________________________________________________________________
//

#ifndef ExcitationGeneratorXmlReader_h
#define ExcitationGeneratorXmlReader_h

#include "excitationGenerator.hxx"
#include <string>


class ExcitationGeneratorXmlReader {

public:
    ExcitationGeneratorXmlReader(const std::string& filename);
    std::auto_ptr<excitationGenerator> getExcitationGenerator();
    friend std::ostream& operator<< (std::ostream& output, const ExcitationGeneratorXmlReader& b);
private:
    std::auto_ptr<excitationGenerator> excitationGenPointer_;
};

#endif
