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
 * Author(s): C. Pizzolato                                                    *
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

#include "ceinms/ContactModelXmlReader.h"
#include <iostream>
using std::cout;
using std::endl;
#include <string>
using std::string;
#include "boost/filesystem.hpp"
using boost::filesystem::absolute;
using boost::filesystem::canonical;

/*
constexpr const char * const strend(const char * const str) {
return *str ? strend(str + 1) : str;
}

constexpr const char * const fromlastslash(const char * const start, const char * const end) {
    return (end >= start && *end != '/') ? fromlastslash(start, end - 1) : (end + 1);
}

constexpr const char * const pathlast(const char * const path) {
    return fromlastslash(path, strend(path));
}
*/

int main() {
    //    cout << pathlast(__FILE__) << endl;
    //    boost::filesystem::path p(__FILE__);
    //    boost::filesystem::path dir = p.parent_path() /= "TestData";
    //    string filename(absolute(dir /= "contactModelKnee.xml").string());
    //    cout << filename << endl;

    //to fix when constexpr works on visual studio
    string filename("C:/Users/S2849511/coding/versioning/ceinms/src/Tests/xmlInput/TestData/contactModelKnee.xml");
    ceinms::ContactModelXmlReader contactModelReader(filename);
    switch (contactModelReader.getContactModelType()) {
    case ceinms::ContactModelType::OpenSim:
        cout << "OpenSim generic contact model" << endl;
        break;
    case ceinms::ContactModelType::KneeBalance:
        cout << "Knee balance contact model" << endl;
        auto setupKnee = contactModelReader.getContactModelSetupKneeBalance();
        cout << "- Intercondyle distance " << setupKnee.intercondyleDistance << endl;
        cout << "- Condyle: " << setupKnee.condyle1.name << endl;
        cout << "----- Dof: " << setupKnee.condyle1.dof << endl;
        cout << "- Condyle: " << setupKnee.condyle2.name << endl;
        cout << "----- Dof: " << setupKnee.condyle2.dof << endl;
        break;
    }
    return 0;
}
