//__________________________________________________________________________
// Author(s): Claudio Pizzolato
//            Elena Ceseracciu
// email:  claudio.pizzolato@griffithuni.edu.au
//
// DO NOT REDISTRIBUTE WITHOUT PERMISSION
//__________________________________________________________________________
//

#ifndef ceinms_FileUtils_h
#define ceinms_FileUtils_h

#include <string>

namespace ceinms {
    namespace FileUtils {

        std::string getFile(const std::string& aPath, const std::string& mask);
        bool isAbsolute(const char *path);
        void trim(std::string& fileName);

    }
}

#endif
