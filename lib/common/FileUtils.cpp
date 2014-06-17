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
#include <iostream>
#include "FileUtils.h"

std::string FileUtils::getFile ( const std::string& aPath, const std::string& mask ) {

    boost::filesystem::path currentPath(aPath);
    currentPath/=mask;
    std::string foundFile;
    if(boost::filesystem::exists(currentPath) &&  boost::filesystem::is_regular_file(currentPath))
        foundFile=currentPath.string();

    if(foundFile.empty()) {
        std::cout << "file found in " + aPath << std::endl;
        exit(EXIT_FAILURE);
    }
    std::cout << "File: " +foundFile << std::endl;
    return foundFile;
}
