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

#include <vector>
using std::vector;
#include <string>
using std::string;
#include <algorithm>
using std::copy;
#include <iostream>
using std::cout;
using std::endl;
#include <stdlib.h>

//#include "DoF.h"
#include "MTU.h"
#include "Curve.h"

namespace ceinms {

    template<typename Activation, typename Tendon, CurveMode::Mode mode>
    DoF<Activation, Tendon, mode>::DoF()
        :id_(""), muscles_(0), momentArms_(0), torque_(0) {  }

    template<typename Activation, typename Tendon, CurveMode::Mode mode>
    DoF<Activation, Tendon, mode>::DoF(const string& id)
        : id_(id), muscles_(0), momentArms_(0), torque_(0) {  }

    template<typename Activation, typename Tendon, CurveMode::Mode mode>
    DoF<Activation, Tendon, mode>::DoF(const DoF& orig)
        : id_(orig.id_), muscles_(orig.muscles_),
        momentArms_(orig.momentArms_), torque_(orig.torque_) {  }


    template<typename Activation, typename Tendon, CurveMode::Mode mode>
    DoF<Activation, Tendon, mode>::~DoF() {
    }

    /*!
      Add a new muscle to the list of muscles connected to the current dof
      */
    template<typename Activation, typename Tendon, CurveMode::Mode mode>
    void DoF<Activation, Tendon, mode>::addNewMuscle(const vectorMTUitr newMuscle) {

        muscles_.push_back(newMuscle);
    }


    // :TODO: probabilmente qui, invece di passargli una copia del vettore,
    //        è meglio passargli il vettore originale con i limiti che deve
    //        copiare


    /*!
      .

      */
    template<typename Activation, typename Tendon, CurveMode::Mode mode>
    void DoF<Activation, Tendon, mode>::setMomentArms(const vector<double>& momentArms){

        if (momentArms.size() != muscles_.size())  {
            std::cout << "We have " << momentArms.size() << " ma data for "
                << muscles_.size() << "muscles.\n";
            std::cout << "Something went wrong, gotta exit!\n";
            exit(EXIT_FAILURE);
        }
        momentArms_.clear();
        momentArms_.resize(muscles_.size());
        copy(momentArms.begin(), momentArms.end(), momentArms_.begin());
    }


    /*!
      Compute the torque <a href="./biblio.html">[2]</a>:
      \f[
      M^{j}(\theta,t)=\sum_{i=1}^m (r_i(\theta) \cdot F_i^{mt}(\theta,t))
      \f]

      where \f$M^j\f$ is the torque of the j-th degree of freedom, \f$r_i(\theta)\f$ and
      \f$F_i^{mt}(\theta,t)\f$ are the moment arm and the musculotendon force of the i-th muscle.

      In the following code:

      \f$\mathtt{torque\_} = M^{j}(\theta,t)\f$,

      \f$\mathtt{muscles\_.at(i)->getMuscleTendonForce()} =F_i^{mt}(\theta,t)\f$,

      \f$\mathtt{*currentMomentArm} = r_i(\theta)\f$

      */

    template<typename Activation, typename Tendon, CurveMode::Mode mode>
    void DoF<Activation, Tendon, mode>::updateTorque() {

        torque_ = 0;
        vector<double>::const_iterator currentMomentArm;
        currentMomentArm = momentArms_.begin();
        for (unsigned int i = 0; i < muscles_.size(); ++i) {
            torque_ += muscles_.at(i)->getMuscleForce() * (*currentMomentArm);
            currentMomentArm++;
        }
    }


    template<typename Activation, typename Tendon, CurveMode::Mode mode>
    bool DoF<Activation, Tendon, mode>::compareMusclesNames(const vector<string>& muscleNames) const  {

        vector<string>::const_iterator m1 = muscleNames.begin();
        for (typename vector<vectorMTUitr>::const_iterator m2 = muscles_.begin();
            m2 != muscles_.end();) {
            if (!(*m2)->compareMusclesId(*m1))
                return false;
            m1++; m2++;
        }

        return true;
    }


    template<typename Activation, typename Tendon, CurveMode::Mode mode>
    void DoF<Activation, Tendon, mode>::getMusclesNamesOnDof(std::vector<std::string>& musclesNames) const
    {
        musclesNames.clear();
        for (unsigned int i = 0; i < muscles_.size(); ++i)
            musclesNames.push_back(muscles_.at(i)->getMuscleName());
    }



    template<typename Activation, typename Tendon, CurveMode::Mode mode>
    std::ostream& operator<< (std::ostream& output, const DoF<Activation, Tendon, mode>& dof)
    {
        output << "Name: " << dof.id_ << std::endl;
        output << "Muscles: ";
        for (unsigned int i = 0; i < dof.muscles_.size(); ++i) {
            dof.muscles_[i]->printName(output);
            output << " ";
        }
        return output;
    }

}
