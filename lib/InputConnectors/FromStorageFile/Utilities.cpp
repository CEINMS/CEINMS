//__________________________________________________________________________
// Author(s): Monica Reggiani - June 2014
// email:  monica.reggiani@gmail.com
//
// DO NOT REDISTRIBUTE WITHOUT PERMISSION
//__________________________________________________________________________
//

#include <iostream>
#include <boost/filesystem.hpp>
#include <vector>
using std::vector;
#include <string>
using std::string; 
#include <iostream>
using std::cout;
using std::endl; 

#include "Utilities.h"

string findFile(const string& pattern, const string& directoryPath) {
  namespace fs =boost::filesystem; 
  
  if ( !fs::exists(directoryPath) ) {
    cout << "findFile: Directory: " << directoryPath << "  does not exist\n";  
    exit(EXIT_FAILURE); 
  }
  
  vector<string> filenames; 
  
  for(fs::directory_iterator it(directoryPath); it != fs::directory_iterator(); ++it)
    filenames.push_back((it->path().filename().string())); 
  
  auto pos = std::remove_if(std::begin(filenames), std::end(filenames),  [&](std::string& s) { return s.find(pattern) == std::string::npos ; }) ;

  filenames.erase(pos, std::end(filenames));

  if (filenames.size() == 0) {
    cout << "Could not find the * "<< pattern << " file in" << directoryPath << endl;
    exit(EXIT_FAILURE);
  }
  if (filenames.size() > 1) {
    cout << "Too many *" <<pattern << " files in" << directoryPath << endl;
    exit(EXIT_FAILURE);
  }
    
  return (filenames.at(0));
}


vector<std::size_t> findMapping(const vector<string>& subset, const vector<string>& set) {
  vector<std::size_t> mapping;
  
  for (auto& it:subset) {
    auto posIt = std::find(set.begin(), set.end(), it);
    if (posIt == set.end()) {
      cout << "ERROR: " << it << " not found in file \n";
      exit(EXIT_FAILURE); 
    }
    std::size_t pos = std::distance(set.begin(), posIt);  
    mapping.push_back(pos);
  }
    
  return mapping;
  
}