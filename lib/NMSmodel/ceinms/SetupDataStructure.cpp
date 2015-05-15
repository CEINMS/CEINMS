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

#include <iostream>
using std::cout;
using std::endl;
#include <vector>
using std::vector;
#include <string>
using std::string;
#include <stdio.h>
#include <fstream>

#include "NMSmodel.h"
#include "MTU.h"
#include "DoF.h"
#include "ceinms/Curve.h"
#include "ceinms/SubjectXmlReader.h"
#include "subject.hxx"
using SubjectXsd::SubjectType;
using SubjectXsd::PointsSequenceType;
using SubjectXsd::MTUDefaultType;
using SubjectXsd::MTUSetType;
using SubjectXsd::MTUType;
using SubjectXsd::MTUNameSetType;
using SubjectXsd::AngleInputType;
using SubjectXsd::DoFSetType;
using SubjectXsd::DoFType;




//TODO: most of this stuff should be moved in SubjectXmlReader
namespace ceinms {

    template<typename NMSmodelT>
    SetupDataStructure<NMSmodelT>::SetupDataStructure(const string& configurationFile)
    {
        SubjectXmlReader sxr(configurationFile);
        subjectPointer_ = sxr.getSubject();
    }


    template<typename NMSmodelT>
    void SetupDataStructure<NMSmodelT>::createCurves() {

        SubjectType::mtuDefault_type& mtuDefault(subjectPointer_->mtuDefault());
        MTUDefaultType::curve_sequence& curveSequence(mtuDefault.curve());
        for (MTUDefaultType::curve_iterator i = curveSequence.begin(); i != curveSequence.end(); ++i) {
            // each i is a curve
            string curveName = (*i).name();
            cout << curveName << endl;
            vector<double> x;

            PointsSequenceType xPoints = (*i).xPoints();
            PointsSequenceType::iterator pointsIt;

            for (pointsIt = xPoints.begin(); pointsIt != xPoints.end(); ++pointsIt) {
                double currentX = (*pointsIt);
                x.push_back(currentX);
            }

            vector<double> y;

            PointsSequenceType yPoints = (*i).yPoints();

            for (pointsIt = yPoints.begin(); pointsIt != yPoints.end(); ++pointsIt) {
                double currentY = (*pointsIt);
                y.push_back(currentY);
            }

            if (curveName == "activeForceLength")  activeForceLengthCurve_.resetPointsWith(x, y);
            if (curveName == "passiveForceLength") passiveForceLengthCurve_.resetPointsWith(x, y);
            if (curveName == "forceVelocity")      forceVelocityCurve_.resetPointsWith(x, y);
            if (curveName == "tendonForceStrain")      tendonForceStrain_.resetPointsWith(x, y);
        }

    }


    template<typename NMSmodelT>
    void SetupDataStructure<NMSmodelT>::createMuscles(NMSmodelT& mySubject) {


        SubjectType::mtuDefault_type& mtuDefault(subjectPointer_->mtuDefault());
        SubjectType::mtuSet_type& mtuSet(subjectPointer_->mtuSet());
        MTUSetType::mtu_sequence& mtuSequence(mtuSet.mtu());
        MTUSetType::mtu_iterator i;
        for (i = mtuSequence.begin(); i != mtuSequence.end(); ++i) {
            string muscleName = (*i).name();
            MTUT newMuscle(muscleName);
            MTUType::emDelay_optional& emDelayOpt((*i).emDelay());
            double emDelay;
            if (emDelayOpt.present())
                emDelay = (*i).emDelay().get();
            else
                emDelay = mtuDefault.emDelay();
            double percentageChange = mtuDefault.percentageChange();
            MTUType::damping_optional& dampingOpt((*i).damping());
            double damping;
            if (dampingOpt.present())
                damping = (*i).damping().get();
            else
                damping = mtuDefault.damping();
            double maxIsometricForce = (*i).maxIsometricForce();
            double c1 = (*i).c1();
            double c2 = (*i).c2();
            double shapeFactor = (*i).shapeFactor();
            double optimalFiberLength = (*i).optimalFibreLength();

            MTUType::pennationAngle_type& xmlPennAngle((*i).pennationAngle());
            double pennationAngle = xmlPennAngle;
            switch (xmlPennAngle.unit()) {
            case AngleInputType::unit_type::deg:
                cout << "Converted " << muscleName << " pennation angle from degrees to radians: " << pennationAngle << " -> ";
                pennationAngle = radians(pennationAngle);
                cout << pennationAngle << endl;
                break;
            default:
                cout << "Assuming " << muscleName << " pennation angle in radians: " << pennationAngle << endl;
                break;
            }
            double tendonSlackLength = (*i).tendonSlackLength();
            double strengthCoefficient = (*i).strengthCoefficient();
            newMuscle.setParametersToComputeActivation(c1, c2, shapeFactor);
            newMuscle.setCurves(activeForceLengthCurve_, passiveForceLengthCurve_, forceVelocityCurve_, tendonForceStrain_);
            if ((*i).maxContractionVelocity().present())
            {
                double maxContractionVelocity = (*i).maxContractionVelocity().get();
                newMuscle.setParametersToComputeForces(optimalFiberLength, pennationAngle,
                    tendonSlackLength, percentageChange, damping, maxIsometricForce, strengthCoefficient, maxContractionVelocity);
            }
            else // let MTU use default value for maxContractionVelocity
            {
                newMuscle.setParametersToComputeForces(optimalFiberLength, pennationAngle,
                    tendonSlackLength, percentageChange, damping, maxIsometricForce, strengthCoefficient);
            }
            newMuscle.setEmDelay(emDelay);
            mySubject.addMuscle(newMuscle);
        }
    }


    template<typename NMSmodelT>
    void SetupDataStructure<NMSmodelT>::createDoFs(NMSmodelT& mySubject) {

        SubjectType::dofSet_type& dofSet(subjectPointer_->dofSet());
        DoFSetType::dof_sequence& dofSequence(dofSet.dof());
        DoFSetType::dof_iterator i;
        for (i = dofSequence.begin(); i != dofSequence.end(); ++i) {

            string dofName = (*i).name();
            DoFT newDoF(dofName);

            // Now have a look at the muscles we have
            MTUNameSetType currentSet = (*i).mtuNameSet();
            MTUNameSetType::iterator mtuNameIt;

            for (mtuNameIt = currentSet.begin(); mtuNameIt != currentSet.end(); ++mtuNameIt) {
                string currentMuscle(*mtuNameIt);

                vectorMTUitrT found;
                if (!mySubject.haveThisMuscle(currentMuscle, found)) {
                    cout << currentMuscle << " not configured. Sorry, we have to exit!\n";
                    exit(EXIT_FAILURE);
                }
                else
                    newDoF.addNewMuscle(found);
            }
            mySubject.addDoF(newDoF);
        }
    }


    template<typename NMSmodelT>
    void SetupDataStructure<NMSmodelT>::writeXMLCalibratedFile(NMSmodelT& mySubject, const string& XMLfilename) {

        SubjectType::mtuDefault_type& mtuDefault(subjectPointer_->mtuDefault());
        SubjectType::mtuSet_type& mtuSet(subjectPointer_->mtuSet());
        MTUSetType::mtu_sequence& mtuSequence(mtuSet.mtu());

        for (MTUSetType::mtu_iterator i(mtuSequence.begin()); i != mtuSequence.end(); ++i) {
            string muscleName = (*i).name();
            MTUT currentMuscle;
            mySubject.getMuscle(currentMuscle, muscleName);
            (*i).c1(currentMuscle.getC1());
            (*i).c2(currentMuscle.getC2());
            (*i).shapeFactor(currentMuscle.getShapeFactor());
            (*i).optimalFibreLength(currentMuscle.getOptimalFiberLength());
            (*i).pennationAngle(currentMuscle.getPennationAngle());
            (*i).tendonSlackLength(currentMuscle.getTendonSlackLength());
            mtuDefault.percentageChange(currentMuscle.getPercentageChange());
            mtuDefault.damping(currentMuscle.getDamping());
            mtuDefault.emDelay(currentMuscle.getEmDelay());
            //  (*i).emDelay(currentMuscle.getEmDelay());
            //NOTE: single emDelay not available yet
            (*i).maxIsometricForce(currentMuscle.getMaxIsometricForce());
            (*i).strengthCoefficient(currentMuscle.getStrengthCoefficient());
        }

        xml_schema::namespace_infomap map;
        map[""].name = "";
        map[""].schema = "subject.xsd";

        std::ofstream ofs(XMLfilename.c_str());
        subject(ofs, *subjectPointer_, map);

    }
}
