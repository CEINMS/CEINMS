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
 * Author(s): C. Pizzolato, L. Tagliapietra                                   *
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

#include "ceinms/Curve.h"
using namespace ceinms;
#include <vector>
#include <fstream>
#include <string>
#include <cmath>
#include "ceinms/NMSmodel.h"
#include "ceinms/Activation/ExponentialActivation.h"
#include "ceinms/Tendon/ElasticTendon_BiSec.h"
using namespace std;
using MtuCurve = Curve<CurveMode::Mode::Offline, CurveMode::Interpolation::Cubic>;


MtuCurve getActiveForceLengthCurve() {

    MtuCurve activeForceLength;
    std::vector<double> afX{ 0.4035, 0.52725, 0.62875, 0.71875, 0.86125, 1.045, 1.2175, 1.43875, 1.61875 };
    std::vector<double> afY{ 0, 0.226667, 0.636667, 0.856667, 0.95, 0.993333, 0.77, 0.246667, 0, };
    activeForceLength.resetPointsWith(afX, afY);
    return activeForceLength;
}

MtuCurve getPassiveForceLengthCurve() {

    MtuCurve passiveForceLength;
    std::vector<double> pfX{ 1, 1.1, 1.2, 1.3, 1.4, 1.5, 1.6};
    std::vector<double> pfY{ 0, 0.035, 0.12, 0.26, 0.55, 1.17, 2};
    passiveForceLength.resetPointsWith(pfX, pfY);
    return passiveForceLength;
}


MtuCurve getForceVelocityCurve() {
    
    MtuCurve forceVelocity;
    std::vector<double> fvX{ -1, -0.6, -0.3, -0.1, 0, 0.1, 0.3, 0.6, 0.8 };
    std::vector<double> fvY{ 0, 0.08, 0.2, 0.55, 1, 1.4, 1.6, 1.7, 1.75 };
    forceVelocity.resetPointsWith(fvX, fvY);
    return forceVelocity;
}

MtuCurve getTendonForceLengthCurve() {

    MtuCurve tendonForceLength;
    std::vector<double> tfX{ 0, 0.00131, 0.00281, 0.00431, 0.00581, 0.00731, 0.00881, 0.0103, 0.0118, 0.0123, 9.2};
    std::vector<double> tfY{0, 0.0108, 0.0257, 0.0435, 0.0652, 0.0915, 0.123, 0.161, 0.208, 0.227, 345};
    tendonForceLength.resetPointsWith(tfX, tfY);
    return tendonForceLength;
}

std::vector<double> getRange(double min, double max, unsigned nPoints) {

    std::vector<double> x;
    double dx = (max - min) / static_cast<double>(nPoints - 1);
    for (unsigned i(0); i < nPoints; ++i)
        x.push_back(min + static_cast<double>(i)*dx);
    return x;
}


void printVectors(const vector<vector<double>>& data, const vector<std::string>& headers, const std::string& filename) {

    std::ofstream oF(filename);
    for (auto h : headers)
        oF << h << ",";
    oF << std::endl;

    int nCols(data.size());
    int nRows(data.front().size());
    for (int iRow(0); iRow < nRows; ++iRow){
        for (int iCol(0); iCol < nCols; ++iCol) 
            oF << data.at(iCol).at(iRow) << ",";
        oF << std::endl;
    }
    oF.close();

}

template<typename T>
void testPassiveCurve(T& mtu) {

    std::vector<double> fl, fv, mf, tl, mtlData, timeColumn;
    double time(0.);
    for (auto mtl : getRange(0.5, 1.5, 1000)) {
        mtu.setMuscleTendonLength(mtl);
        mtu.setEmg(0);
        mtu.setTime(time += 0.00);
        mtu.updateActivation();
        mtu.updateFibreLengthAndVelocity();
        mtu.updateMuscleForce();
        mtu.pushState();
        timeColumn.emplace_back(time);
        mtlData.emplace_back(mtl);
        fl.emplace_back(mtu.getNormFiberLength());
        fv.emplace_back(mtu.getNormFiberVelocity());
        tl.emplace_back(mtu.getTendonLength());
        mf.emplace_back(mtu.getMuscleForce());
    }
    printVectors({ timeColumn, mtlData, fl, fv, tl, mf }, { "time", "mtl", "fl", "fv", "tl", "mf" }, "passiveCurve.csv");

}


template<typename T>
void testEmgStepResponse(T& mtu) {

    std::vector<double> fl, fv, mf, tl, mtlData, timeColumn, pa;
    double time(0.);
    double emg(0.);
    for (auto mtl : getRange(1, 1, 50)) {
        mtu.setMuscleTendonLength(mtl);
        if (time > 1)
            emg += 0.01;
        mtu.setEmg(emg);        
        mtu.setTime(time += 0.1);
        mtu.updateActivation();
        mtu.updateFibreLengthAndVelocity();
        mtu.updateMuscleForce();
        mtu.pushState();
        timeColumn.emplace_back(time);
        mtlData.emplace_back(mtl);
        fl.emplace_back(mtu.getFiberLength());
        fv.emplace_back(mtu.getFiberVelocity());
    //    cout << mtu.getFiberVelocity() << endl;
        pa.emplace_back(mtu.getPennationAngleAtT());
        tl.emplace_back(mtu.getTendonLength());
        mf.emplace_back(mtu.getMuscleForce());
    }
    printVectors({ timeColumn, mtlData, fl, pa, fv, tl, mf }, { "time", "mtl", "fl", "pa", "fv", "tl", "mf" }, "emgStep.csv");

}



template<typename T>
void testRamp(T& mtu) {

    std::vector<double> fl, fv, mf, tl, mtlData, timeColumn,pa;
    double time(0.);
    for (auto mtl : getRange(0.668, 1.5, 1000)) {
        mtu.setMuscleTendonLength(mtl);
        mtu.setEmg(1);
        time += 0.001;
        mtu.setTime(time);
        mtu.updateActivation();
        mtu.updateFibreLengthAndVelocity();
        mtu.updateMuscleForce();
        mtu.pushState();
        timeColumn.emplace_back(time);
        mtlData.emplace_back(mtl);
        fl.emplace_back(mtu.getFiberLength());
        fv.emplace_back(mtu.getFiberVelocity());
        tl.emplace_back(mtu.getTendonLength());
        mf.emplace_back(mtu.getMuscleForce());
        pa.emplace_back(mtu.getPennationAngleAtT());
    }
    printVectors({ timeColumn, mtlData, fl, pa, fv, tl, mf }, { "time", "mtl", "fl", "pa", "fv", "tl", "mf" }, "mtlRamp.csv");

}


template <typename T>
void printCurve(const T& curve, const std::string& filename, double startX, double endX, unsigned nPoints = 1000) {

    std::ofstream oF(filename);
    double increment((endX - startX) / static_cast<double>(nPoints));
    for (unsigned i(0); i <= nPoints; ++i) {
        double x = startX + i*increment;
        oF << x << "," << curve.getValue(x) << std::endl;
    }
    oF.close();

}

void testCurve1() {

    std::vector<double> y{ 0.371234, 0.371234, 0.371234, 0.371234, 0.371329, 0.372146, 0.372989, 0.373819, 0.374651, 0.375484 };
    std::vector<double> x{ 0.05, 0.1, 0.15, 0.2, 0.25, 0.3, 0.35, 0.4, 0.45, 0.5 };
    Curve < CurveMode::Mode::Offline, CurveMode::Interpolation::Cubic, 10 > c;
    for (int i(0); i < y.size(); ++i) {
        c.addPoint(x.at(i), y.at(i));
        string name("curveT" + std::to_string(i) + ".csv");
        printCurve(c, name, x.front(), x.back(), 100);
    }


}

int main() {


    MTU<ExponentialActivation, ElasticTendon_BiSec, CurveMode::Mode::Online> mtu("muscle");
    mtu.setCurves(
        getActiveForceLengthCurve(),
        getPassiveForceLengthCurve(),
        getForceVelocityCurve(),
        getTendonForceLengthCurve());
    
    mtu.setParametersToComputeActivation(-0.5, -0.5, -1);
    mtu.setParametersToComputeForces(0.17, 0.13, 0.83, 0.15, 0.1, 1, 1);
    testRamp(mtu);
    testEmgStepResponse(mtu);
    testCurve1();

    return 0;

}
