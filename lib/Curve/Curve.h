// This is part of
// NeuroMuscoloSkeletal Model Software (NMS)
// Copyright (C) 2010 David Lloyd Massimo Sartori Monica Reggiani
//
// ?? Licenza ??
//
// The authors may be contacted via:
// email: massimo.sartori@gmail.com monica.reggiani@gmail.com

#ifndef Curve_h
#define Curve_h

#include <vector>
#include "CircularVector.h"

namespace CurveMode {
    enum Mode{Online, Offline};
};


template <CurveMode::Mode mode, typename T, typename U>
struct Select {

   typedef T Result;
};


template <typename T, typename U>
struct Select<CurveMode::Online, T, U> {

   typedef U Result;
}; 


template <CurveMode::Mode mode>
class Curve;


template <CurveMode::Mode mode>
std::ostream& operator<< (std::ostream& output, const Curve<mode>& c);


template <CurveMode::Mode mode>
class Curve {

public:
    Curve();
    // compute coefficients
    Curve(const std::vector<double>& x, const std::vector<double> &y);
    Curve(const Curve& orig);
    virtual ~Curve() {}
    Curve& operator=(const Curve& orig);
    void reset();
    // add a new points and compute again coefficients
    void addPoint(double x, double y);
    void addPointOnly(double x, double y);
    void resetPointsWith(const std::vector<double>& x, const std::vector<double> &y);
    void refresh(); 
    // remove last point of the Curve_c and compute again coefficients
    void removeLastPoint();
    void removeLastPointNoUpdate(); //remove last point without computing the coefficients again
    // interpolation 
    double getValue(double xValue) const;
    double getFirstDerivative(double xValue) const;
    double getSecondDerivative(double xValue) const;
    friend std::ostream& operator<< <>(std::ostream& output, const Curve& c);
    bool empty() {return b_.empty();}
private:
    void computeCoefficients();

    typedef typename Select<mode, std::vector<double>, CircularVector<double> >::Result VectorType;  
    VectorType x_;
    VectorType y_;
    std::vector<double>    b_;
    std::vector<double>    c_;
    std::vector<double>    d_;

};

#include "Curve.cpp"

#endif
