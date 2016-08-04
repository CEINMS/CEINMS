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

#ifndef ceinms_Dof_h
#define ceinms_Dof_h

#include <vector>
#include <list>
#include <string>
#include "MTU.h"
#include "ceinms/Curve.h"

namespace ceinms {

    template<typename Activation, typename Tendon, CurveMode::Mode mode>
    class DoF;

    template<typename Activation, typename Tendon, CurveMode::Mode mode>
    std::ostream& operator<< (std::ostream& output, const DoF<Activation, Tendon, mode>& dof);
    /** This class associates muscles to the corrisponding degree of freedom
     *  and compute the torque
     */
    template<typename Activation, typename Tendon, CurveMode::Mode mode>
    class DoF {
    public:

        typedef MTU<Activation, Tendon, mode> MTUtype;
        typedef typename std::vector< MTU<Activation, Tendon, mode> >::iterator vectorMTUitr;
        DoF();
        DoF(const std::string& id);
        DoF(const DoF& orig);
        virtual ~DoF();
        std::string getName() const { return id_; }
        void addNewMuscle(const typename std::vector<MTUtype>::iterator newMuscle);
        friend std::ostream& operator<< <>(std::ostream& output, const DoF& dof);
        void setMomentArms(const std::vector<double>& momentArms);
        void getMomentArms(std::vector<double>& momentArms) const { momentArms = momentArms_; }
        double getTorque() const { return torque_; }
        double getDofStiffness() const { return dofStiffness_;}
        bool compareMusclesNames(const std::vector<std::string>& muscleNames) const;
        void getMusclesNamesOnDof(std::vector<std::string>& musclesNames) const;
        void updateTorque();
        void updateDofStiffness();
    private:
        std::string id_;
        std::vector<  typename std::vector< MTUtype >::iterator > muscles_;
        std::vector<double> momentArms_;
        double torque_;
        double dofStiffness_;
    };
}
#include "DoF.cpp"

#endif
