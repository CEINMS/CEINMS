//__________________________________________________________________________
// Author(s): Claudio Pizzolato
//            Elena Ceseracciu
// email:  claudio.pizzolato@griffithuni.edu.au
//
// DO NOT REDISTRIBUTE WITHOUT PERMISSION
//__________________________________________________________________________
//

#include <string>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#include <iostream>
#include "FileUtils.h"

namespace ceinms {
    namespace FileUtils {

        std::string getFile(const std::string& aPath, const std::string& mask) {

            boost::filesystem::path currentPath(aPath);
            currentPath /= mask;
            std::string foundFile;
            if (boost::filesystem::exists(currentPath) && boost::filesystem::is_regular_file(currentPath))
                foundFile = currentPath.string();

            if (foundFile.empty()) {
                std::cout << "file found in " + aPath << std::endl;
                exit(EXIT_FAILURE);
            }
            std::cout << "File: " + foundFile << std::endl;
            return foundFile;
        }


        bool isAbsolute(const char *path) {
            if (path[0] == '/' || path[0] == '\\') {
                return true;
            }
            std::string str(path);
            if (str.length() > 1) {
                if (str[1] == ':') {
                    return true;
                }
            }
            return false;
        }

        void trim(std::string& fileName)
        {
            boost::trim(fileName);
            boost::trim_if(fileName, boost::is_any_of("\""));
        }

    }
}