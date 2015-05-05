/* -------------------------------------------------------------------------- *
 * CEINMS is a standalone toolbox for neuromusculoskeletal modelling and      *
 * simulation. CEINMS can also be used as a plugin for OpenSim either         *
 * through the OpenSim GUI or API. See https://simtk.org/home/ceinms and the  *
 * NOTICE file for more information. CEINMS development was coordinated       *
 * through Griffith University and supported by the Australian National       *
 * Health and Medical Research Council (NHMRC), the US National Institutes of *
 * Health (NIH), and the European Union Framework Programme 7 (EU FP7). Also  *
 * see the PROJECTS file for more information about the funding projects.     *
 *                                                                            *
 * Copyright (c) 2010-2015 Griffith University and the Contributors           *
 *                                                                            *
 * CEINMS Contributors: C. Pizzolato, M. Reggiani, M. Sartori,                *
 *                      E. Ceseracciu, and D.G. Lloyd                         *
 *                                                                            *
 * Author(s): M. Reggiani                                                     *
 *                                                                            *
 * CEINMS is licensed under the Apache License, Version 2.0 (the "License").  *
 * You may not use this file except in compliance with the License. You may   *
 * obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0.*
 *                                                                            *
 * Unless required by applicable law or agreed to in writing, software        *
 * distributed under the License is distributed on an "AS IS" BASIS,          *
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.   *
 * See the License for the specific language governing permissions and        *
 * limitations under the License.                                             *
 * -------------------------------------------------------------------------- */

#include "Utilities.h"

#include <iostream>
#include <boost/filesystem.hpp>
#include <vector>
using std::vector;
#include <string>
using std::string;
#include <iostream>
using std::cout;
using std::endl;

namespace ceinms {
    string findFile(const string& pattern, const string& directoryPath) {
        namespace fs = boost::filesystem;

        if (!fs::exists(directoryPath)) {
            cout << "findFile: Directory: " << directoryPath << "  does not exist\n";
            exit(EXIT_FAILURE);
        }

        vector<string> filenames;

        for (fs::directory_iterator it(directoryPath); it != fs::directory_iterator(); ++it)
            filenames.push_back((it->path().filename().string()));

        auto pos = std::remove_if(std::begin(filenames), std::end(filenames), [&](std::string& s) { return s.find(pattern) == std::string::npos; });

        filenames.erase(pos, std::end(filenames));

        if (filenames.size() == 0) {
            cout << "Could not find the * " << pattern << " file in" << directoryPath << endl;
            exit(EXIT_FAILURE);
        }
        if (filenames.size() > 1) {
            cout << "Too many *" << pattern << " files in" << directoryPath << endl;
            exit(EXIT_FAILURE);
        }

        return (filenames.at(0));
    }


    vector<std::size_t> findMapping(const vector<string>& subset, const vector<string>& set) {
        vector<std::size_t> mapping;

        for (auto& it : subset) {
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
}
