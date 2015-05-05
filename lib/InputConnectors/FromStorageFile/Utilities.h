//__________________________________________________________________________
// Author(s): Monica Reggiani - June 2014
// email:  monica.reggiani@gmail.com
//
// DO NOT REDISTRIBUTE WITHOUT PERMISSION
//__________________________________________________________________________
//

#ifndef ceinms_Utilities_h
#define ceinms_Utilities_h

#include <vector>
#include <string>

namespace ceinms {
    std::string findFile(const std::string& pattern, const std::string& directoryPath);
    std::vector<std::size_t> findMapping(const std::vector<std::string>& subset, const std::vector<std::string>& set);
}

#endif