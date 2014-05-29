//__________________________________________________________________________
// Author(s): Elena Ceseracciu - May 2014
// email:  elena.ceseracciu@gmail.com
//
// DO NOT REDISTRIBUTE WITHOUT PERMISSION
//__________________________________________________________________________
//

#ifndef SubjectXmlReader_h
#define SubjectXmlReader_h

#include "subject.hxx"
#include <string>


class SubjectXmlReader {

public:
    SubjectXmlReader(const std::string& filename);
    std::auto_ptr<SubjectType> getSubject();
    friend std::ostream& operator<< (std::ostream& output, const SubjectXmlReader& b);
private:
    std::auto_ptr<SubjectType> subjectPointer_;
};

#endif