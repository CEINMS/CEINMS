#include "Curve.h"
#include <vector>
#include <fstream>
#include <string>
#define _USE_MATH_DEFINES
#include <math.h>


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
        curve.addPointOnly(x, std::forward<Lambda>(function)(x));
    }
    curve.refresh();
   
}




void testMuscleCurves() {

     Curve<CurveMode::Mode::Offline, CurveMode::Interpolation::Cubic> activeForceLength, passiveForceLength, forceVelocity, tendonForceLength;

    std::vector<double> afX{ -5, 0, 0.401, 0.402, 0.4035, 0.52725, 0.62875, 0.71875, 0.86125, 1.045, 1.2175, 1.43875, 1.61875, 1.62, 1.621, 2.2, 5 };
    std::vector<double> afY{ 0, 0, 0, 0, 0, 0.226667, 0.636667, 0.856667, 0.95, 0.993333, 0.77, 0.246667, 0, 0, 0, 0, 0 };
    activeForceLength.resetPointsWith(afX, afY);
    printCurve(activeForceLength, "af.txt", -5, 5);


    std::vector<double> pfX{ -5, 0.998, 0.999, 1, 1.1, 1.2, 1.3, 1.4, 1.5, 1.6, 1.601, 1.602, 5 };
    std::vector<double> pfY{ 0, 0, 0, 0, 0.035, 0.12, 0.26, 0.55, 1.17, 2, 2, 2, 2 };
    passiveForceLength.resetPointsWith(pfX, pfY);
    printCurve(passiveForceLength, "pf.txt", -5, 5);

    std::vector<double> fvX{ -10, -1, -0.6, -0.3, -0.1, 0, 0.1, 0.3, 0.6, 0.8, 10 };
    std::vector<double> fvY{ 0, 0, 0.08, 0.2, 0.55, 1, 1.4, 1.6, 1.7, 1.75, 1.75 };
    forceVelocity.resetPointsWith(fvX, fvY);
    printCurve(forceVelocity, "fv.txt", -10, 10);


    std::vector<double> tfX{ -10, -0.002, -0.001, 0, 0.00131, 0.00281, 0.00431, 0.00581, 0.00731, 0.00881, 0.0103, 0.0118, 0.0123, 9.2, 9.201, 9.202, 20 };
    std::vector<double> tfY{ 0, 0, 0, 0, 0.0108, 0.0257, 0.0435, 0.0652, 0.0915, 0.123, 0.161, 0.208, 0.227, 345, 345, 345, 345 };
    tendonForceLength.resetPointsWith(tfX, tfY);
    printCurve(tendonForceLength, "tf.txt", -10, 20);
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
    return 0;

}