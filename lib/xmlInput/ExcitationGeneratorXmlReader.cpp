//__________________________________________________________________________
// Author(s): Elena Ceseracciu - June 2014
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

#include "ExcitationGeneratorXmlReader.h"
#include "validation.h"
#include "excitationGenerator-schema.hxx"
using ExcitationGeneratorXsd::excitationGenerator;

namespace ceinms {
    ExcitationGeneratorXmlReader::ExcitationGeneratorXmlReader(const string& filename)
    {
        try {
            excitationGenPointer_ = parseAndValidate<excitationGenerator>(filename, excitationGenerator_schema, sizeof(excitationGenerator_schema));
        }
        catch (const xml_schema::exception& e) {
            cout << e << endl;
            exit(EXIT_FAILURE);
        }
    }

    std::auto_ptr< excitationGenerator > ExcitationGeneratorXmlReader::getExcitationGenerator()
    {
        return excitationGenPointer_;
    }
}