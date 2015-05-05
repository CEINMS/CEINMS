//__________________________________________________________________________
// Author(s): Elena Ceseracciu - May 2014
// email:  elena.ceseracciu@gmail.com
//
// DO NOT REDISTRIBUTE WITHOUT PERMISSION
//__________________________________________________________________________
//


#include <string>
using std::string;
#include <iostream>
using std::cout;
using std::endl;

#include "SubjectXmlReader.h"
#include "validation.h"
#include "subject-schema.hxx"
using SubjectXsd::SubjectType;

namespace ceinms {
    SubjectXmlReader::SubjectXmlReader(const std::string& filename)
    {

        try {
            subjectPointer_ = parseAndValidate<SubjectType>(filename, subject_schema, sizeof(subject_schema));
        }
        catch (const xml_schema::exception& e) {
            cout << e << endl;
            exit(EXIT_FAILURE);
        }

    }

    std::auto_ptr<SubjectType> SubjectXmlReader::getSubject()
    {
        return subjectPointer_;
    }
}