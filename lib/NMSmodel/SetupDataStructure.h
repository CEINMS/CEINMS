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

#ifndef ceinms_SetupDataStructure_h
#define ceinms_SetupDataStructure_h

#include "subject.hxx" // TODO: we should find a way to only include this from xmlInput
#include "Curve.h"
#include <cmath>
#include <string>

namespace ceinms {
    template<typename NMSmodelT>
    class SetupDataStructure{
    public:
        typedef typename NMSmodelT::MTUtype MTUT;
        typedef typename NMSmodelT::DoFtype DoFT;
        typedef typename NMSmodelT::vectorMTUitr vectorMTUitrT;
        typedef Curve<CurveMode::Offline> CurveOffline;
        SetupDataStructure(const std::string& inputFile);
        void  createCurves();
        void  createMuscles(NMSmodelT& mySubject);
        void  createDoFs(NMSmodelT& mySubject);
        void  writeXMLCalibratedFile(NMSmodelT& mySubject, const std::string& XMLfilename);
    private:
        // Create the body structure from the Input
        std::auto_ptr<SubjectXsd::SubjectType> subjectPointer_;
        CurveOffline forceVelocityCurve_;
        CurveOffline activeForceLengthCurve_;
        CurveOffline passiveForceLengthCurve_;
        CurveOffline tendonForceStrain_;
    };

    //non-member non-friend functions are the best!
    template <typename NMSmodelT>
    void setupSubject(NMSmodelT& mySubject, std::string subjectFile, double tolerance = 1e-6) {

        SetupDataStructure<NMSmodelT> setupData(subjectFile);
        setupData.createCurves();
        setupData.createMuscles(mySubject);
        setupData.createDoFs(mySubject);
        mySubject.setTendonTolerance(tolerance);
    }

}

#include "SetupDataStructure.cpp"
#endif
