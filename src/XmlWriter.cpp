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
 * Author(s): C. Pizzolato, M. Reggiani                                       *
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
using std::string;
#include "subject.hxx"
using SubjectXsd::CalibrationInfoType;
using SubjectXsd::CalibratedType;
using SubjectXsd::SubjectType;

#include <fstream>
#include <iostream>
#include <list>
//#include "XmlWriter.h"
namespace ceinms {
    //TODO: it should re-parse all the info from the subject, instead of re-writing the oldXml
    XmlWriter::XmlWriter(const string& startXmlFile)
        :outFilename_("calibratedSubject.xml"), startXmlFile_(startXmlFile)
    {
        SubjectXmlReader sxr(startXmlFile);
        subjectPointer_ = sxr.getSubject();

    }


    void XmlWriter::setOutputFilename(const string& outFilename) {

        outFilename_ = outFilename;
    }


    void XmlWriter::addCalibrationInfo(const std::string& calibrationXmlFilename) {

        CalibrationInfoType::uncalibrated_optional& uncalibratedOpt(subjectPointer_->calibrationInfo().uncalibrated());
        if (uncalibratedOpt.present()) {
            uncalibratedOpt.reset();
            CalibratedType::calibrationSequence_type myCalibrationSequence;
            myCalibrationSequence.push_back(calibrationXmlFilename);
            CalibratedType::startSubjectFile_type myStartSubject(startXmlFile_);
            CalibratedType myCalibrated(myStartSubject, myCalibrationSequence);
            subjectPointer_->calibrationInfo().calibrated(myCalibrated);
        }
        else {
            SubjectType::calibrationInfo_type& myCalibrationInfo(subjectPointer_->calibrationInfo());
            SubjectType::calibrationInfo_type::calibrated_type::calibrationSequence_type& myCalibrationSequence(myCalibrationInfo.calibrated()->calibrationSequence());
            myCalibrationSequence.push_back(calibrationXmlFilename);
        }
    }


    template <typename NMSmodelT>
    void XmlWriter::setParameters(NMSmodelT& subject) {


        SubjectType::mtuDefault_type& mtuDefault(subjectPointer_->mtuDefault());
        SubjectType::mtuSet_type& mtuSet(subjectPointer_->mtuSet());
        MTUSetType::mtu_sequence& mtuSequence(mtuSet.mtu());

        for (MTUSetType::mtu_iterator i(mtuSequence.begin()); i != mtuSequence.end(); ++i) {
            string muscleName = (*i).name();
            typename NMSmodelT::MTUtype currentMuscle;
            subject.getMuscle(currentMuscle, muscleName);
            (*i).c1(currentMuscle.getC1());
            (*i).c2(currentMuscle.getC2());
            (*i).shapeFactor(currentMuscle.getShapeFactor());
            (*i).optimalFibreLength(currentMuscle.getOptimalFibreLength());
            (*i).pennationAngle(currentMuscle.getPennationAngle());
            (*i).tendonSlackLength(currentMuscle.getTendonSlackLength());
            (*i).maxContractionVelocity(currentMuscle.getMaxContractionVelocity());
            mtuDefault.percentageChange(currentMuscle.getPercentageChange());
            mtuDefault.damping(currentMuscle.getDamping());
            mtuDefault.emDelay(currentMuscle.getEmDelay());
            //  (*i).emDelay(currentMuscle.getEmDelay());
            //NOTE: single emDelay not available yet
            (*i).maxIsometricForce(currentMuscle.getMaxIsometricForce());
            (*i).strengthCoefficient(currentMuscle.getStrengthCoefficient());
        }


    }


    void XmlWriter::write() {

        xml_schema::namespace_infomap map;
        map[""].name = "";
        map[""].schema = "subject.xsd";

        std::ofstream ofs(outFilename_.c_str());
        subject(ofs, *subjectPointer_, map);
    }

}
