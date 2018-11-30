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

#ifndef ceinms_GeneralisedCoordinatesData_h
#define ceinms_GeneralisedCoordinatesData_h

#include <vector>
#include "rtosim/GeneralisedCoordinatesData.h"
//TODO: add checks for different vecotr sizes and stuff
namespace ceinms {
  /*  class GeneralisedCoordinatesData {
    private:
        size_t nCoordinates_;
        std::vector<double> q_, qd_, qdd_;

    public:

        GeneralisedCoordinatesData() :
            nCoordinates_(0) {}

        GeneralisedCoordinatesData(size_t nCoordinates) :
            nCoordinates_(nCoordinates),
            q_(nCoordinates, .0),
            qd_(nCoordinates, .0),
            qdd_(nCoordinates, .0)
        {}

        size_t getNCoordinates() const { return nCoordinates_; }

        void setQ(const std::vector<double>& q) {
            nCoordinates_ = q.size();
            q_ = q;
        }
        void setQd(const std::vector<double>& qd) {
            qd_ = qd;
        }
        void setQdd(const std::vector<double>& qdd) {
            qdd_ = qdd;
        }

        const std::vector<double>& getQ() const { return q_; }
        const std::vector<double>& getQd() const { return qd_; }
        const std::vector<double>& getQdd() const { return qdd_; }
        void getQ(std::vector<double>& q) const { q = q_; }
        void getQd(std::vector<double>& qd) const { qd = qd_; }
        void getQdd(std::vector<double>& qdd) const { qdd = qdd_; }
    };
	*/
	using GeneralisedCoordinatesData = rtosim::GeneralisedCoordinatesData;
}
#endif
