#ifndef FileUtils_h
#define FileUtils_h

#include <string>
#include <boost/filesystem.hpp>
#include <iostream>

class FileUtils {
public:
static std::string getFile ( const std::string& aPath, const std::string& mask ) {

    boost::filesystem::path currentPath(aPath);
    if(!boost::filesystem::exists(currentPath) || !boost::filesystem::is_directory(currentPath)) {
        std::cout << aPath + " not found.\n";
        exit(EXIT_FAILURE);
    }
    std::string foundFile;
    boost::filesystem::directory_iterator endIt;
    for(boost::filesystem::directory_iterator dirIt(currentPath); dirIt != endIt; ++dirIt) {
        if(boost::filesystem::is_regular_file(dirIt->status())) {
            std::string filename(dirIt->path().filename().string());
            if(filename.find(mask) != std::string::npos)
                foundFile = filename;                    
        }
    }
    
    if(foundFile.empty()) {
        std::cout << "file found in " + aPath << std::endl;
        exit(EXIT_FAILURE);
    }
    std::cout << "File: " +foundFile << std::endl;
    return foundFile;
	}
};

#endif