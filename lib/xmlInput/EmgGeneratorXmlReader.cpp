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

#include "EmgGeneratorXmlReader.h"
#include "validation.h"
#include "emgGenerator-schema.hxx"

EmgGeneratorXmlReader::EmgGeneratorXmlReader(const string& filename)
{
    try {
    emgGenPointer_= parseAndValidate<emgGenerator>(filename, emgGenerator_schema, sizeof(emgGenerator_schema));
    }
    catch (const xml_schema::exception& e) {
        cout << e << endl;
        exit(EXIT_FAILURE);
    }
}

std::auto_ptr< emgGenerator > EmgGeneratorXmlReader::getEmgGenerator()
{
    return emgGenPointer_;
}
