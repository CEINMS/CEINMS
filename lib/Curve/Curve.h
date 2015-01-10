//__________________________________________________________________________
// Author(s): Claudio Pizzolato, Monica Reggiani - September 2013
// email:  claudio.pizzolato@griffithuni.edu.au
//         monica.reggiani@gmail.com
//
// DO NOT REDISTRIBUTE WITHOUT PERMISSION
//__________________________________________________________________________
//

#ifndef Curve_h
#define Curve_h

#include <vector>
#include "CircularVector.h"

namespace CurveMode {
    enum Mode{Online, Offline};
    enum Interpolation{Cubic, Linear};

    
};


template <int v>
struct Int2Type {

    enum { value = v };
};


template <CurveMode::Mode mode, typename T, typename U>
struct Select {

   typedef T Result;
};


template <typename T, typename U>
struct Select<CurveMode::Online, T, U> {

   typedef U Result;
}; 


template <CurveMode::Mode mode,  CurveMode::Interpolation T>
class Curve;


template <CurveMode::Mode mode,  CurveMode::Interpolation T>
std::ostream& operator<< (std::ostream& output, const Curve<mode, T>& c);


template <CurveMode::Mode mode, CurveMode::Interpolation T = CurveMode::Cubic>
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
    bool addPoint(double x, double y);
    bool addPointOnly(double x, double y);
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
    void computeCoefficients(Int2Type<CurveMode::Cubic>);
    void computeCoefficients(Int2Type<CurveMode::Linear>);

    double getValue(double xValue, unsigned abscissaPoint, Int2Type<CurveMode::Cubic>) const;
    double getValue(double xValue, unsigned abscissaPoint, Int2Type<CurveMode::Linear>) const;
    unsigned getAbscissaPoint(double xValue) const;

    typedef typename Select<mode, std::vector<double>, CircularVector<double> >::Result VectorType;  
    VectorType x_;
    VectorType y_;
    std::vector<double>    b_;
    std::vector<double>    c_;
    std::vector<double>    d_;

};

#include "Curve.cpp"

#endif
