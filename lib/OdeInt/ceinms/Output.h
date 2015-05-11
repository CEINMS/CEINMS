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
 
#ifndef Output_h
#define Output_h

#include <vector>

struct Output {

    int kMax_;
    int nVar_;
    int nSave_;
    bool dense_;
    int count_;
    double x1_, x2_, xOut_, dxOut_;
    std::vector<double> xSave_;
    std::vector< std::vector<double> > ySave_;
    Output() : kMax_(-1), dense_(false), count_(0) {}

    Output(int nSave) : kMax_(500), nSave_(nSave), count_(0), xSave_(500) {
        dense_ = nSave > 0 ? true : false;
    }

    void init(const int nEqn, const double xLo, const double xHi) {
        nVar_ = nEqn;
        if (kMax_ == -1) return;
        ySave_.resize(nVar_, std::vector<double>(kMax_));
//        for(unsigned i = 0; i < nVar_; ++i)
//            ySave_.at(i).resize(kMax_);
        if (dense_) {
            x1_    = xLo;
            x2_    = xHi;
            xOut_  = x1_;
            dxOut_ = (x2_ - x1_)/nSave_;
        }

    }

    void resize() {
        int kOld = kMax_;
        kMax_ *= 2;
        std::vector<double> tempVec(xSave_);
        xSave_.resize(kMax_);
        for (unsigned k = 0; k < kOld; ++k)
        xSave_.at(k) = tempVec.at(k);
        std::vector< std::vector<double> > tempMat(ySave_);
        ySave_.resize(nVar_, std::vector<double>(kMax_));
        for (unsigned i = 0; i < nVar_; ++i)
            for (unsigned k = 0; k < kOld; ++k)
                ySave_.at(i).at(k) = tempMat.at(i).at(k);
    }

    template <class Stepper>
    void save_dense(Stepper& s, const double xout, const double h) {

        if (count_ == kMax_) resize();
        for (unsigned i = 0; i < nVar_;i++)
            ySave_.at(i).at(count_) = s.dense_out(i,xout,h);
        xSave_.at(count_++) = xout;
    }


    void save(double x, const std::vector<double> &y) {

        if (kMax_ <= 0) return;
        if (count_ == kMax_) resize();
        for (unsigned i = 0; i < nVar_; ++i)
            ySave_.at(i).at(count_) = y.at(i);
        xSave_.at(count_++) = x;
    }


    template <class Stepper>
    void out(const int nStp, const double x, const std::vector<double>& y, Stepper& s, const double h) {

        if (!dense_)
            throw("dense output not set in Output!");
        if (nStp == -1) {
            save(x,y);
            xOut_ += dxOut_;
        } else {
            while ((x - xOut_)*(x2_ - x1_) > 0.0) {
            save_dense(s, xOut_,h);
            xOut_ += dxOut_;
            }
        }
    }

};


#endif
