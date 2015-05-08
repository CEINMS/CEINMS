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
 * Author(s):                                                                 *
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
 
#ifndef Odeint_h
#define Odeint_h

#include <vector>
#include <limits>
#include <math.h>

#include "Output.h"

inline float sign(const double &a, const double &b)
        {return b >= 0 ? (a >= 0 ? a : -a) : (a >= 0 ? -a : a);}


template<typename Stepper>
class Odeint {

public:
    Odeint(std::vector<double>& yStart, const double x1, const double x2,
           const double aTol, const double rTol, const double h1,
           const double hMin, Output& out, typename Stepper::Dtype &derivs);
    void integrate();

private:
    static const unsigned MAXSTP_ = 50000;
    double EPS_;
    int nok_;
    int nbad_;
    int nvar_;
    double x1_, x2_, hMin_;
    bool dense_;
    std::vector<double> y_, dydx_;
    std::vector<double> &yStart_;
    Output &out_;
    typename Stepper::Dtype &derivs_;
    Stepper s_;
    unsigned nstp_;
    double x_, h_;
};




template<typename Stepper>
Odeint<Stepper>::Odeint(std::vector<double>& yStart, const double x1, const double x2,
                        const double aTol, const double rTol, const double h1,
                        const double hMin, Output& out, typename Stepper::Dtype &derivs):
nvar_(yStart.size()),
y_(yStart.size()),
dydx_(yStart.size()),
yStart_(yStart),
x_(x1),
nok_(0),
nbad_(0),
x1_(x1),
x2_(x2),
hMin_(hMin),
dense_(out.dense_),
out_(out),
derivs_(derivs),
s_(y_, dydx_, x_, aTol, rTol, out.dense_) {

    EPS_ = std::numeric_limits<double>::epsilon();
    h_ = sign(h1, x2-x1);
    for (unsigned i = 0; i < nvar_; ++i) y_.at(i) = yStart.at(i);
    out_.init(s_.nEqn_, x1_, x2_);
}



template<class Stepper>
void Odeint<Stepper>::integrate() {

    derivs_(x_, y_, dydx_);
    if (dense_)
        out_.out(-1, x_, y_, s_, h_);
    else
        out_.save(x_, y_);
    for(unsigned nstp = 0; nstp < MAXSTP_; ++nstp) {

        if((x_+h_*1.0001 - x2_)*(x2_ - x1_) > 0.0)
            h_ = x2_ - x_;
            s_.step(h_, derivs_);

        if(s_.hDid_ == h_) ++nok_; else ++nbad_;

        if(dense_)
            out_.out(nstp_, x_, y_, s_, s_.hDid_);
        else
            out_.save(x_, y_);

        if ((x_ - x2_)*(x2_ - x1_) >= 0.0) {
            for(unsigned i = 0; i < nvar_; i++) yStart_.at(i) = y_.at(i);
            if (out_.kMax_ > 0 && fabs(out_.xSave_.at(out_.count_-1) - x2_) > 100.0*fabs(x2_)*EPS_)
                out_.save(x_,y_);
            return;
        }

        if (fabs(s_.hNext_) <= hMin_) throw("Step size too small in Odeint");
        h_ = s_.hNext_;
    }
    throw("Too many steps in routine Odeint");
}

#endif
