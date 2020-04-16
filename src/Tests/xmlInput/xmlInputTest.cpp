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

#include "ceinms/NMSmodel.h"
#include "ceinms/Activation/ExponentialActivation.h"
#include "ceinms/Tendon/ElasticTendon_BiSec.h"
#include "ceinms/Curve.h"
#include "ceinms/ContactModelXmlReader.h"
#include "ceinms/ExecutionXmlReader.h"
#include "ceinms/XmlWriter.h"
#include "ceinms/CalibrationXmlReader.h"
#include <sstream>
#include <iostream>
using std::cout;
using std::endl;
#include <string>
using std::string;
#include "boost/filesystem.hpp"
using boost::filesystem::absolute;
using boost::filesystem::canonical;
using boost::filesystem::path;

void testContactModels(std::string filename) {
    
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
}


void testExecutionXmlReader(std::string filename) {
      
    auto print([](std::vector<std::string> v) {
        for (auto& e : v) {
            cout << e << " ";
        }
        cout << "\n";
    });
    ceinms::ExecutionXmlReader executionReader(filename);
    std::vector<std::string> v;
    executionReader.getDofSet(v);
    cout << "DofSet\n";
    print(v);
    v.clear();
    executionReader.getMusclesToAdjust(v);
    cout << "MusclesToAdjust\n";
    print(v);
    v.clear();
    executionReader.getMusclesToSynthesise(v);
    cout << "MusclesToSynthesise\n";
    print(v);
    v.clear();
}

void testSubjectXmlFile(std::string inFilename, string outFilename) {

	ceinms::NMSmodel<ceinms::ExponentialActivation, ceinms::ElasticTendon_BiSec, ceinms::CurveMode::Offline> subject1, subject2;
	ceinms::setupSubject(subject1, inFilename);
	ceinms::XmlWriter writer(inFilename);
	writer.setParameters(subject1);
	writer.setOutputFilename(outFilename);
	writer.write();
	ceinms::setupSubject(subject2, outFilename);
	std::stringstream ss1, ss2;
	ss1 << subject1;
	ss2 << subject2;
	if (ss1.str() == ss2.str())
		cout << "Subject XML re-read produce the same result" << endl;
	else
		cout << "Subject XML re-read FAILED" << endl;
	cout << subject1;
}

void testCalibrationXmlReader(std::string filename) {

    ceinms::CalibrationXmlReader reader(filename);
    cout << reader;
}

int main() {
    boost::filesystem::path dir = path(BASE_DIR) /= "TestData";
    testContactModels((dir / "contactModelKnee.xml").string());
    testExecutionXmlReader((dir / "executionHybrid.xml").string());
	testSubjectXmlFile((dir / "subject.xml").string(), (dir / "outSubject.xml").string());
    testCalibrationXmlReader((dir / "deprecatedCalibrationCfg.xml").string());
    testCalibrationXmlReader((dir / "calibrationCfg1.xml").string());
    testCalibrationXmlReader((dir / "calibrationCfg2.xml").string());
    return 0;
}
