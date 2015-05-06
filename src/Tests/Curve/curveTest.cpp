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
 * Author(s): C. Pizzolato, L. Tagliapietra                                                    *
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

#include "Curve.h"
using namespace ceinms;
#include <vector>
#include <fstream>
#include <string>
#include <cmath>


std::vector<double> getRange(double min, double max, unsigned nPoints) {

    std::vector<double> x;
    double dx = (max - min) / static_cast<double>(nPoints-1);
    for (unsigned i(0); i < nPoints; ++i)
        x.push_back(min + i*dx);
    return x;
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


template <typename T>
void printCurveDerivative(const T& curve, const std::string& filename, double startX, double endX, unsigned nPoints = 1000) {

    std::ofstream oF(filename);
    double increment((endX - startX) / static_cast<double>(nPoints));
    for (unsigned i(0); i <= nPoints; ++i) {
        double x = startX + i*increment;
        oF << x << "," << curve.getFirstDerivative(x) << std::endl;
    }
    oF.close();

}


template <typename T, typename U>
void compareCurve(const T& curve, U&& function, const std::string& filename, double startX, double endX, unsigned nPoints = 1000) {

    std::ofstream oF(filename);
    oF << "x,y1,y2\n";
    auto xVals = getRange(startX, endX, nPoints);

    for (auto x : xVals)
        oF << x << "," << curve.getValue(x) << "," << function(x) << std::endl;

    oF.close();

}


template<typename Lambda, typename CurveT>
void createCurve(std::vector<double> xPoints, Lambda&& function, CurveT& curve){

    curve.reset();
    for (auto x : xPoints) {
        curve.addPointNoUpdate(x, std::forward<Lambda>(function)(x));
    }
    curve.refresh();

}


void testOriginalMuscleCurves() {

    Curve<CurveMode::Mode::Offline, CurveMode::Interpolation::Cubic> activeForceLength, passiveForceLength, forceVelocity, tendonForceLength;

    std::vector<double> afX{ -5, 0, 0.401, 0.402, 0.4035, 0.52725, 0.62875, 0.71875, 0.86125, 1.045, 1.2175, 1.43875, 1.61875, 1.62, 1.621, 2.2, 5 };
    std::vector<double> afY{ 0, 0, 0, 0, 0, 0.226667, 0.636667, 0.856667, 0.95, 0.993333, 0.77, 0.246667, 0, 0, 0, 0, 0 };
    activeForceLength.resetPointsWith(afX, afY);
    printCurve(activeForceLength, "flc_original.txt", -5, 5);

    std::vector<double> pfX{ -5, 0.998, 0.999, 1, 1.1, 1.2, 1.3, 1.4, 1.5, 1.6, 1.601, 1.602, 5 };
    std::vector<double> pfY{ 0, 0, 0, 0, 0.035, 0.12, 0.26, 0.55, 1.17, 2, 2, 2, 2 };
    passiveForceLength.resetPointsWith(pfX, pfY);
    printCurve(passiveForceLength, "pflc_original.txt", -5, 5);

    std::vector<double> fvX{ -10, -1, -0.6, -0.3, -0.1, 0, 0.1, 0.3, 0.6, 0.8, 10 };
    std::vector<double> fvY{ 0, 0, 0.08, 0.2, 0.55, 1, 1.4, 1.6, 1.7, 1.75, 1.75 };
    forceVelocity.resetPointsWith(fvX, fvY);
    printCurve(forceVelocity, "fvc_original.txt", -10, 10);

    std::vector<double> tfX{ -10, -0.002, -0.001, 0, 0.00131, 0.00281, 0.00431, 0.00581, 0.00731, 0.00881, 0.0103, 0.0118, 0.0123, 9.2, 9.201, 9.202, 20 };
    std::vector<double> tfY{ 0, 0, 0, 0, 0.0108, 0.0257, 0.0435, 0.0652, 0.0915, 0.123, 0.161, 0.208, 0.227, 345, 345, 345, 345 };
    tendonForceLength.resetPointsWith(tfX, tfY);
    printCurve(tendonForceLength, "tflc_original.txt", -10, 20);
}


void testMillardMuscleCurves() {

    Curve<CurveMode::Mode::Offline, CurveMode::Interpolation::Cubic> activeForceLength, passiveForceLength, forceVelocity, tendonForceLength;

    std::vector<double> afX{ 0.1441, 0.1841, 0.2241, 0.2641, 0.3041, 0.3441, 0.3841, 0.4241, 0.4641, 0.5041, 0.5441, 0.5841, 0.6241, 0.6641, 0.7041, 0.7441, 0.7841, 0.8241, 0.8641, 0.9041, 0.9441, 0.9841, 1.0241, 1.0641, 1.1041, 1.1441, 1.1841, 1.2241, 1.2641, 1.3041, 1.3441, 1.3841, 1.4241, 1.4641, 1.5041, 1.5441, 1.5841,1.6241, 1.6641, 1.7041, 1.7441, 1.7841, 1.8241, 1.8641, 1.9041, 1.9441, 1.9841, 2.0241, 2.0641, 2.1041 };
    std::vector<double> afY{ 0, 0, 0, 0, 0, 0, 0, 0, 0.0036, 0.1127, 0.2531, 0.3948, 0.5362, 0.6732, 0.7739, 0.8204, 0.8568, 0.8915, 0.9260, 0.9602, 0.9919, 1.0000, 0.9997, 0.9907, 0.9524, 0.9004, 0.8457, 0.7902, 0.7343, 0.6783, 0.6222, 0.5661, 0.5099, 0.4537, 0.3976, 0.3414, 0.2853, 0.2292, 0.1732, 0.1175, 0.0623, 0.0117, 0, 0, 0, 0, 0, 0, 0, 0 };
    activeForceLength.resetPointsWith(afX, afY);
    printCurve(activeForceLength, "flc_Millard.txt", 0.14, 2.1);

    std::vector<double> pfX{ 0.7000, 0.7260, 0.7520, 0.7780, 0.8040, 0.8300, 0.8560, 0.8820, 0.9080, 0.9340, 0.9600, 0.9860, 1.0120, 1.0380, 1.0640, 1.0900, 1.1160, 1.1420, 1.1680, 1.1940, 1.2200, 1.2460, 1.2720, 1.2980, 1.3240, 1.3500, 1.3760, 1.4020, 1.4280, 1.4540, 1.4800, 1.5060, 1.5320, 1.5580, 1.5840, 1.6100, 1.6360, 1.6620, 1.6880, 1.7140, 1.7400, 1.7660, 1.7920, 1.8180, 1.8440, 1.8700, 1.8960, 1.9220, 1.9480, 1.9740, 2.0000 };
    std::vector<double> pfY{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0.0002, 0.0041, 0.0093, 0.0148, 0.0208, 0.0278, 0.0362, 0.0469, 0.0604, 0.0775, 0.0986, 0.1240, 0.1536, 0.1872, 0.2248, 0.2662, 0.3117, 0.3614, 0.4155, 0.4742, 0.5371, 0.6037, 0.6732, 0.7448, 0.8178, 0.8915, 0.9657, 1.0400, 1.1143, 1.1886, 1.2629, 1.3371, 1.4114, 1.4857, 1.5600, 1.6343, 1.7086, 1.7829, 1.8571};
    passiveForceLength.resetPointsWith(pfX, pfY);
    printCurve(passiveForceLength, "pflc_Millard.txt", 0.7, 2.0);

    std::vector<double> fvX{ -1.3000, -1.2480, -1.1960, -1.1440, -1.0920, -1.0400, -0.9880, -0.9360, -0.8840, -0.8320, -0.7800, -0.7280, -0.6760, -0.6240, -0.5720, -0.5200, -0.4680, -0.4160, -0.3640, -0.3120, -0.2600, -0.2080, -0.1560, -0.1040, -0.0520, -0.0000,  0.0520, 0.1040, 0.1560, 0.2080, 0.2600, 0.3120, 0.3640, 0.4160, 0.4680, 0.5200, 0.5720, 0.6240, 0.6760, 0.7280, 0.7800, 0.8320, 0.8840, 0.9360, 0.9880, 1.0400, 1.0920, 1.1440, 1.1960, 1.2480, 1.3000};
    std::vector<double> fvY{ 0 , 0, 0, 0, 0, 0, 0, 0.0048, 0.0165, 0.0296, 0.0432, 0.0579, 0.0744, 0.0939, 0.1173, 0.1457, 0.1798, 0.2198, 0.2657, 0.3174, 0.3754, 0.4409, 0.5179, 0.6162, 0.7645, 1.0000, 1.1756, 1.2278, 1.2541, 1.2719, 1.2856, 1.2972, 1.3075, 1.3170, 1.3258, 1.3344, 1.3426, 1.3507, 1.3587, 1.3666, 1.3745, 1.3823, 1.3901, 1.3977, 1.4000, 1.4000, 1.4000, 1.4000, 1.4000, 1.4000, 1.4000};
    forceVelocity.resetPointsWith(fvX, fvY);
    printCurve(forceVelocity, "fvc_Millard.txt", -1.3, 1.3);

    std::vector<double> tfX{ -0.20, -0.18, -0.16, -0.14, -0.12, -0.10, -0.08, -0.06, -0.04, -0.02, 0.00, 0.02, 0.04, 0.06, 0.08, 0.10, 0.12, 0.14, 0.16, 0.18, 0.20, 0.22, 0.24, 0.26, 0.28, 0.30, 0.32, 0.34, 0.36, 0.38, 0.40, 0.42, 0.44, 0.46, 0.48, 0.50 };
    std::vector<double> tfY{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0.0, 0.2609, 0.7474, 1.3087, 1.8699, 2.4311, 2.9924, 3.5536, 4.1148, 4.6760, 5.2372, 5.7985, 6.3597, 6.9209, 7.4821, 8.0434, 8.6046, 9.1658, 9.7270, 10.2883, 10.8495, 11.4107, 11.9719, 12.5332, 13.0944, 13.6556};
    tendonForceLength.resetPointsWith(tfX, tfY);
    printCurve(tendonForceLength, "tflc_Millard.txt", -0.2, 0.5);
}


void testCircularVector() {

    CircularVector<double, 10> cv10;
    std::cout << "Circular vector size 10" << std::endl;
    std::cout << "1 - store numbers from 0 to 9" << std::endl;
    for (auto x : getRange(0, 9, 10))
        cv10.push_back(x);
    std::cout << cv10 << std::endl;
    std::cout << "2 - push number 10 in the vector" << std::endl
              << "    the vector should print numbers from 1 to 10" << std::endl;
    cv10.push_back(10);
    std::cout << cv10 << std::endl;
    std::cout << "3 - remove last entry" << std::endl
              << "    the vector should print numbers from 1 to 9" << std::endl;
    cv10.pop_back();
    std::cout << cv10 << std::endl;
    std::cout << "4 - clear vector" << std::endl
              << "    the vector should print nothing" << std::endl;
    cv10.clear();
    std::cout << cv10<< std::endl;
    std::cout << "5 - try to remove more elements, even if it is empty" << std::endl
              << "    the vector should print nothing" << std::endl;
    cv10.pop_back();
    std::cout << cv10 << std::endl;
    std::cout << "6 - add 5 elements 1 to 5" << std::endl
              << "    the vector should print numbers from 1 to 5" << std::endl;
    for (auto x : getRange(1, 5, 5))
        cv10.push_back(x);
    std::cout << cv10 << std::endl;
    std::cout << "7 - add 500 elements 1001 to 1500" << std::endl
              << "    the vector should print numbers from 1491 to 1500" << std::endl;
    for (auto x : getRange(1001, 1500, 500))
        cv10.push_back(x);
    std::cout << cv10 << std::endl;
    std::cout << "8 - remove last 3 elements" << std::endl
              << "    the vector should print numbers from 1491 to 1497" << std::endl;
    for (unsigned i(0); i < 3; ++i)
        cv10.pop_back();
    std::cout << cv10 << std::endl;

    std::cout << "9 - ask for size" << std::endl
              << "    size should be equal 7" << std::endl;
    std::cout << cv10.size() << std::endl;

}


int main() {

    using MyCurve = Curve < CurveMode::Mode::Online, CurveMode::Interpolation::Cubic, 3 > ;
    std::vector<double> x(getRange(-M_PI, M_PI, 10));

    MyCurve sinCurve;
    auto sinFun([](double x){return std::sin(x); });
    auto cosFun([](double x){return std::cos(x); });
    createCurve(getRange(-M_PI, M_PI, 100), sinFun, sinCurve);
    compareCurve(sinCurve, sinFun,  "sin.txt", -4, 4);
    printCurveDerivative(sinCurve, "sinD.txt", -4, 4);


    std::ofstream oF("lastPointDerivative.txt");
    oF << "x,y\'_curve,y\'\n";
    auto xRange(getRange(-M_PI, M_PI, 100));
    MyCurve sinCurve2;
    for (auto x : xRange) {
        sinCurve2.addPoint(x, std::sin(x));
        oF << x << "," << sinCurve2.getFirstDerivative(x) << ","<< std::cos(x) << std::endl;
    }
    oF.close();

    MyCurve lineCurve;
    auto lineFun([](double x){
        double m = 1, q = 0;
        return m*x + q;
    });
    createCurve(getRange(-1, 1, 2), lineFun, lineCurve);
    compareCurve(lineCurve, lineFun, "line.txt", -4, 4);
    printCurveDerivative(lineCurve, "lineD.txt", -4, 4);

    testCircularVector();

    testOriginalMuscleCurves();
    testMillardMuscleCurves();
    return 0;

}
