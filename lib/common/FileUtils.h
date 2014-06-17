//__________________________________________________________________________
// Author(s): Claudio Pizzolato
//            Elena Ceseracciu
// email:  claudio.pizzolato@griffithuni.edu.au
//
// DO NOT REDISTRIBUTE WITHOUT PERMISSION
//__________________________________________________________________________
//

#ifndef FileUtils_h
#define FileUtils_h

#include <string>

class FileUtils {
public:
static std::string getFile ( const std::string& aPath, const std::string& mask );
};

#endif
