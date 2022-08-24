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
#include <filesystem>
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>

#include "ceinms/FileUtils.h"

namespace ceinms {

    namespace FileUtils {

        void sortMaFilenames(const std::map<std::string, std::string>& maMap, const std::vector<std::string>& dofNames, std::vector<std::string>& maDataFilenames) {
            int currentDof = 0;
            for (auto& it : dofNames) {
                try {
                    maDataFilenames.push_back(maMap.at(it));
                }
                catch (std::out_of_range) {
                    std::cerr << "Could not find moment arm file for " << it << " degree of freedom" << std::endl;
                    exit(EXIT_FAILURE);
                }
            }
        }

        std::string getFile(const std::string& aPath, const std::string& mask) {

            std::filesystem::path currentPath(aPath);
            currentPath /= mask;
            std::string foundFile;
            if (std::filesystem::exists(currentPath) && std::filesystem::is_regular_file(currentPath))
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

        bool fileExists(const std::string& filename) {
            return std::filesystem::exists(filename);
        }

        std::string simplify(const std::string& filename) {
            std::string fullpath = filename;
            try {
                fullpath = std::filesystem::canonical(filename).generic_string();

                // fix a problem with boost not replacing the separator correctly in windows
                if (std::filesystem::path::preferred_separator == L'\x5c') {
                    std::replace(fullpath.begin(), fullpath.end(), '/', '\\');
                }
            }
            catch (std::exception& e) {
                std::cout << e.what();
            }
            return fullpath;
        }

        std::string makeGeneric(const std::string& filename) {
            std::string fullpath = filename;
            try {
                fullpath = std::filesystem::path(filename).generic_string();
            }
            catch (std::exception& e) {
                std::cout << e.what();

            }
            return fullpath;
        }

        void trim(std::string& fileName) {
            char* t = " \t\n\r\f\v/";
            fileName.erase(0, fileName.find_first_not_of(t));
            fileName.erase(fileName.find_last_not_of(t) + 1);
        }

        std::string getFilePath(const std::string& filename) {
            // save the location of input filename
            std::string::size_type dirSep = filename.rfind('/'); // Unix/Mac dir separator
            std::string filepath("");
            if (dirSep == std::string::npos)
                dirSep = filename.rfind('\\'); // DOS dir separator

            if (dirSep != std::string::npos) // if '_fileName' contains path information...
                filepath = filename.substr(0, dirSep + 1); // include trailing slashes
            return filepath;
        }
    }
}
