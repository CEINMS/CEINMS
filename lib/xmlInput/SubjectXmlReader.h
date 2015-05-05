//__________________________________________________________________________
// Author(s): Elena Ceseracciu - May 2014
// email:  elena.ceseracciu@gmail.com
//
// DO NOT REDISTRIBUTE WITHOUT PERMISSION
//__________________________________________________________________________
//

#ifndef ceinms_SubjectXmlReader_h
#define ceinms_SubjectXmlReader_h

#include "subject.hxx"
#include <string>

namespace ceinms {
    class SubjectXmlReader {
    public:
        SubjectXmlReader(const std::string& filename);
        std::auto_ptr<SubjectXsd::SubjectType> getSubject();
        friend std::ostream& operator<< (std::ostream& output, const SubjectXmlReader& b);
    private:
        std::auto_ptr<SubjectXsd::SubjectType> subjectPointer_;
    };
}

#endif