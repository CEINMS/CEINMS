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

#include <vector>
using std::vector;

namespace ceinms {
    namespace Hybrid {
        namespace StaticComputationMode {

            template<typename NMSmodelT>
            Default<NMSmodelT>::Default(NMSmodelT& subject) :
                subject_(subject) {

                subject_.getMuscleForces(forceDataT1_);
                cout << "muscle forces\n";
                for (unsigned i = 0; i < forceDataT1_.size(); ++i)
                    cout << forceDataT1_.at(i) << " ";
                cout << endl;


                subject_.getEmgs(emgDataT1_);
                musclesToUpdate_.assign(subject_.getNoMuscles(), true);

            }


            template<typename NMSmodelT>
            void Default<NMSmodelT>::getMusclesToUpdate() {

                musclesToUpdate_.clear();
                vector<double> currentEMGs;
                subject_.getEmgs(currentEMGs);
                for (unsigned idx = 0; idx < currentEMGs.size(); ++idx)
                    if (currentEMGs.at(idx) != emgDataT1_.at(idx))
                        musclesToUpdate_.push_back(idx);
                emgDataT1_ = currentEMGs;
            }



            template<typename NMSmodelT>
            void Default<NMSmodelT>::getTorques(vector< double >& torques) {

                getMusclesToUpdate();
                subject_.updateState_HYBRID(musclesToUpdate_);
                subject_.getTorques(torques);

                //              std::cout << "currentTorques\n";
                //             for (unsigned i = 0; i < torques.size(); ++i)
                //                 std::cout << torques.at(i) << " ";
                //             std::cout << std::endl;


                /*        vector<double> currentEMGs;

                      std::cout << "currentEMGs\n";
                      subject_.getEmgs(currentEMGs);
                      for (unsigned i = 0; i < currentEMGs.size(); ++i)
                      std::cout << currentEMGs.at(i) << " ";
                      std::cout << std::endl;
                      */
            }

        }
    }
}
