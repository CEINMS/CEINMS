#ifndef FileUtils_h
#define FileUtils_h

#include <string>
#include <boost/filesystem.hpp>
#include <iostream>

class FileUtils {
public:
static std::string getFile ( const std::string& aPath, const std::string& mask ) {

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
};

#endif