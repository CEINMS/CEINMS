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
 * Author(s): C. Pizzolato, E. Ceseracciu                                     *
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
