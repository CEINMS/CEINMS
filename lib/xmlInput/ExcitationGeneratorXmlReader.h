//__________________________________________________________________________
// Author(s): Elena Ceseracciu - June 2014
// email:  elena.ceseracciu@gmail.com
//
// DO NOT REDISTRIBUTE WITHOUT PERMISSION
//__________________________________________________________________________
//

#ifndef ceinms_ExcitationGeneratorXmlReader_h
#define ceinms_ExcitationGeneratorXmlReader_h

#include "excitationGenerator.hxx"
#include <string>

namespace ceinms {
    class ExcitationGeneratorXmlReader {

    public:
        ExcitationGeneratorXmlReader(const std::string& filename);
        std::auto_ptr<ExcitationGeneratorXsd::excitationGenerator> getExcitationGenerator();
        friend std::ostream& operator<< (std::ostream& output, const ExcitationGeneratorXmlReader& b);
    private:
        std::auto_ptr<ExcitationGeneratorXsd::excitationGenerator> excitationGenPointer_;
    };
}

#endif