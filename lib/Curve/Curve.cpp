//__________________________________________________________________________
// Author(s): Claudio Pizzolato, Monica Reggiani - September 2013
// email:  claudio.pizzolato@griffithuni.edu.au
//         monica.reggiani@gmail.com
//
// DO NOT REDISTRIBUTE WITHOUT PERMISSION
//__________________________________________________________________________
//

#include "Curve.h"
#include <iostream>
#include <vector>
using std::vector;

#include <algorithm>

namespace ceinms {
    //const static int pointsNumber = 17;
    template <CurveMode::Mode mode, CurveMode::Interpolation T, size_t N>
    Curve<mode, T, N>::Curve() {
        // make a local copy of the points
        x_.clear();
        y_.clear();
        b_.resize(0);
        c_.resize(0);
        d_.resize(0);

    }

    /* CALC_SPLINE_COEEFICIENTS: this routine takes an array of x and y values,
    * and computes the coefficients of natural splines which interpolate the data.
    * The code is translated from a Fortran version printed in "Computer
    * Methods for Mathematical Computations" by Forsythe, Malcolm, and
    * Moler, pp 77-8.
    */

    template <CurveMode::Mode mode, CurveMode::Interpolation T, size_t N>
    Curve<mode, T, N>::Curve(const vector<double>& x, const vector<double> &y) {

        x_ = x;
        y_ = y;

        computeCoefficients(Int2Type<T>());
    }


    template <CurveMode::Mode mode, CurveMode::Interpolation T, size_t N>
    Curve<mode, T, N>::Curve(const Curve<mode, T, N>& orig) {
        x_ = orig.x_;
        y_ = orig.y_;
        b_ = orig.b_;
        c_ = orig.c_;
        d_ = orig.d_;
    };


    template <CurveMode::Mode mode, CurveMode::Interpolation T, size_t N>
    Curve<mode, T, N>& Curve<mode, T, N>::operator=(const Curve<mode, T, N>& orig) {
        x_ = orig.x_;
        y_ = orig.y_;
        b_ = orig.b_;
        c_ = orig.c_;
        d_ = orig.d_;
        return *this;
    }


    template <CurveMode::Mode mode, CurveMode::Interpolation T, size_t N>
    void Curve<mode, T, N>::reset() {

        x_.clear();
        y_.clear();
        b_.resize(0);
        c_.resize(0);
        d_.resize(0);
    }


    template <CurveMode::Mode mode, CurveMode::Interpolation T, size_t N>
    void Curve<mode, T, N>::resetPointsWith(const vector<double>& x, const vector<double> &y) {
        // make a local copy of the points
        x_ = x;
        y_ = y;

        computeCoefficients(Int2Type<T>());
    }


    template <CurveMode::Mode mode, CurveMode::Interpolation T, size_t N>
    void Curve<mode, T, N>::removeLastPoint()
    {
        x_.pop_back();
        y_.pop_back();

        computeCoefficients(Int2Type<T>());
    }


    template <CurveMode::Mode mode, CurveMode::Interpolation T, size_t N>
    void Curve<mode, T, N>::removeLastPointNoUpdate() {

        x_.pop_back();
        y_.pop_back();

    }


    template <CurveMode::Mode mode, CurveMode::Interpolation T, size_t N>
    bool Curve<mode, T, N>::addPoint(double x, double y)  {

        if (x_.empty() || x > x_.back())
        {
            x_.push_back(x);
            y_.push_back(y);

            computeCoefficients(Int2Type<T>());
            return true;
        }
        else return false;
    }


    template <CurveMode::Mode mode, CurveMode::Interpolation T, size_t N>
    bool Curve<mode, T, N>::addPointNoUpdate(double x, double y)
    {
        if (x_.empty() || x > x_.back())
        {
            x_.push_back(x);
            y_.push_back(y);
            return true;
        }
        else return false;
    }


    template <CurveMode::Mode mode, CurveMode::Interpolation T, size_t N>
    void Curve<mode, T, N>::refresh()
    {
        computeCoefficients(Int2Type<T>());
    }


    template <CurveMode::Mode mode, CurveMode::Interpolation T, size_t N>
    void Curve<mode, T, N>::computeCoefficients(Int2Type<CurveMode::Linear>) {

        unsigned n = x_.size();
        b_.resize(n, .0);
        c_.resize(n, .0);
        d_.resize(n, .0);

        if (n >= 2) {
            for (unsigned k = 0; k < n - 1; ++k)
                b_.at(k) = (y_.at(k + 1) - y_.at(k)) / (x_.at(k + 1) - x_.at(k));
            b_.at(n - 1) = b_.at(n - 2);
        }
    }

    template <CurveMode::Mode mode, CurveMode::Interpolation T, size_t N>
    void Curve<mode, T, N>::computeCoefficients(Int2Type<CurveMode::Cubic>) {

        unsigned n = x_.size();
        int nm1 = n - 1;


        b_.resize(n);
        c_.resize(n);
        d_.resize(n);


        if (n > 2) {
            // setup tridiagonal system:
            // b_ = diagonal
            // d_ = offdiagonal
            // c_ = right-hand side
            d_.at(0) = x_.at(1) - x_.at(0);
            c_.at(1) = (y_.at(1) - y_.at(0)) / d_.at(0);

            for (unsigned i = 1; i < nm1; ++i) {
                d_.at(i) = x_.at(i + 1) - x_.at(i);
                b_.at(i) = 2.0*(d_.at(i - 1) + d_.at(i));
                c_.at(i + 1) = (y_.at(i + 1) - y_.at(i)) / d_.at(i);
                c_.at(i) = c_.at(i + 1) - c_.at(i);
            }

            // End conditions. Third derivatives at x_.at(0) and x_.at(n-1)
            // are obtained from divided differences. 

            b_.at(0) = -d_.at(0);
            b_.at(nm1) = -d_.at(nm1 - 1);
            c_.at(0) = 0.;
            c_.at(nm1) = 0.;

            if (n > 3) {
                c_.at(0) = c_.at(2) / (x_.at(3) - x_.at(1)) - c_.at(1) / (x_.at(2) - x_.at(0));
                c_.at(nm1) = c_.at(nm1 - 1) / (x_.at(nm1) - x_.at(n - 3)) -
                    c_.at(n - 3) / (x_.at(nm1 - 1) - x_.at(n - 4));
                c_.at(0) = c_.at(0) * d_.at(0) * d_.at(0) / (x_.at(3) - x_.at(0));
                c_.at(nm1) = -c_.at(nm1) * d_.at(nm1 - 1) * d_.at(nm1 - 1) / (x_.at(nm1) - x_.at(n - 4));
            }

            // Forward elimination 
            for (unsigned i = 1; i < n; ++i) {
                double t = d_.at(i - 1) / b_.at(i - 1);
                b_.at(i) -= t * d_.at(i - 1);
                c_.at(i) -= t * c_.at(i - 1);
            }

            // Back substitution
            c_.at(nm1) /= b_.at(nm1);
            for (int j = 0; j < nm1; ++j) {
                int i = nm1 - 1 - j;
                c_.at(i) = (c_.at(i) - d_.at(i) * c_.at(i + 1)) / b_.at(i);
            }

            // compute polynomial coefficients
            b_.at(nm1) = (y_.at(nm1) - y_.at(nm1 - 1)) / d_.at(nm1 - 1) +
                d_.at(nm1 - 1) * (c_.at(nm1 - 1) + 2.0* c_.at(nm1));

            for (int i = 0; i < nm1; ++i) {
                b_.at(i) = (y_.at(i + 1) - y_.at(i)) / d_.at(i) - d_.at(i) * (c_.at(i + 1) + 2.0 * c_.at(i));
                d_.at(i) = (c_.at(i + 1) - c_.at(i)) / d_.at(i);
                c_.at(i) *= 3.0;
            }

            c_.at(nm1) *= 3.0;
            d_.at(nm1) = d_.at(nm1 - 1);

        }

        if (n == 2) {
            b_.at(0) = (y_.at(1) - y_.at(0)) / (x_.at(1) - x_.at(0));
            b_.at(1) = b_.at(0);
            c_.at(0) = 0.;
            c_.at(1) = 0.;
            d_.at(0) = 0.;
            d_.at(1) = 0.;
        }


        // do nothing if size < 2

    }


    template <CurveMode::Mode mode, CurveMode::Interpolation T, size_t N>
    unsigned Curve<mode, T, N>::getAbscissaPoint(double xValue) const{

        const unsigned n = x_.size();
        unsigned k = 0;
        if (n == 2)
            k = 0;
        else if (xValue <= x_.front()) k = 0;
        else if (xValue >= x_.back()) k = n - 1;
        else k = getAbscissaPoint(xValue, Int2Type<mode>());

        //  std::cout << "k, k1 "<< k << " " <<k1 << std::endl;

        return k;
    }


    //the circular vector does't have iterators.. so in the meantime we fix it like this
    template <CurveMode::Mode mode, CurveMode::Interpolation T, size_t N>
    unsigned Curve<mode, T, N>::getAbscissaPoint(double xValue, Int2Type<CurveMode::Online>) const {

        unsigned i = 0;
        unsigned j = x_.size();
        unsigned k = 0;
        while (1) {
            k = (i + j) / 2;
            if (xValue < x_.at(k))
                j = k;
            else if (xValue > x_.at(k + 1))
                i = k;
            else
                break;
        }
        return k;
    }


    template <CurveMode::Mode mode, CurveMode::Interpolation T, size_t N>
    unsigned Curve<mode, T, N>::getAbscissaPoint(double xValue, Int2Type<CurveMode::Offline>) const {

        return std::distance(x_.begin(), std::lower_bound(x_.begin(), x_.end(), xValue)) - 1;
    }


    template <CurveMode::Mode mode, CurveMode::Interpolation T, size_t N>
    double Curve<mode, T, N>::getValue(double xValue) const {

        unsigned n = x_.size();
        double yValue(.0);
        if (xValue < x_.at(0))
            yValue = y_.at(0);
        else if (xValue > x_.at(n - 1))
            yValue = y_.at(n - 1);
        else if (n == 1)
            yValue = x_.at(0);
        else
            yValue = getValue(xValue, getAbscissaPoint(xValue), Int2Type<T>());

        return yValue;
    }

    /*******************************************************************************/
    /* INTERPOLATE_SPLINE: given a spline function and an x-value, this routine
    * finds the corresponding y-value by interpolating the spline. It
    * can return the zeroth, first, or second derivative of the spline
    * at that x-value.
    */
    template <CurveMode::Mode mode, CurveMode::Interpolation T, size_t N>
    double Curve<mode, T, N>::getValue(double xalue, unsigned abscissaPoint, Int2Type<CurveMode::Cubic>) const {

        const unsigned k(abscissaPoint);
        double dx = xalue - x_.at(k);
        double result(y_.at(k) + dx * (b_.at(k) + dx * (c_.at(k) + dx * d_.at(k))));
        return result;
    }


    template <CurveMode::Mode mode, CurveMode::Interpolation T, size_t N>
    double Curve<mode, T, N>::getValue(double xalue, unsigned abscissaPoint, Int2Type<CurveMode::Linear>) const {

        const unsigned k(abscissaPoint);
        double dx = xalue - x_.at(k);
        return b_.at(k)*dx + y_.at(k);

    }


    /*******************************************************************************/
    /* INTERPOLATE_SPLINE: given a spline function and an x-value, this routine

    * finds the corresponding y-value by interpolating the spline. It
    * can return the zeroth, first, or second derivative of the spline
    * at that x-value.
    */

    template <CurveMode::Mode mode, CurveMode::Interpolation T, size_t N>
    double Curve<mode, T, N>::getFirstDerivative(double xValue) const  {


        int n = x_.size();
        double yValue(.0);
        if (xValue < x_.at(0))
            yValue = b_.at(0);
        else if (xValue > x_.at(n - 1))
            yValue = b_.at(n - 1);
        else if (n == 1)
            yValue = .0;
        else yValue = getFirstDerivative(xValue, getAbscissaPoint(xValue), Int2Type<T>());
        return yValue;

    }


    template <CurveMode::Mode mode, CurveMode::Interpolation T, size_t N>
    double Curve<mode, T, N>::getFirstDerivative(double xValue, unsigned abscissaPoint, Int2Type<CurveMode::Cubic>) const {

        unsigned k(abscissaPoint);
        double dx = xValue - x_.at(k);
        return (b_.at(k) + dx * (2.0 * c_.at(k) + 3.0 * dx * d_.at(k)));

    }


    template <CurveMode::Mode mode, CurveMode::Interpolation T, size_t N>
    double Curve<mode, T, N>::getFirstDerivative(double xValue, unsigned abscissaPoint, Int2Type<CurveMode::Linear>) const {

        return b_.at(abscissaPoint);
    }


    template <CurveMode::Mode mode, CurveMode::Interpolation T, size_t N>
    double Curve<mode, T, N>::getSecondDerivative(double xValue) const  {


        int n = x_.size();


        // Now see if the abscissa is out of range of the function

        if (xValue < x_.at(0))
            return b_.at(0);

        if (xValue > x_.at(n - 1))
            return b_.at(n - 1);


        if (n == 1)
            return 0;

        int k = 0;
        if (n == 2)
            k = 0;
        else {
            // Do a binary search to find which two spline control points the abscissa
            // is between   

            int i = 0;
            int j = n;

            while (1)
            {
                k = (i + j) / 2;
                if (xValue < x_.at(k))
                    j = k;
                else if (xValue > x_.at(k + 1))
                    i = k;
                else
                    break;
            }
        }
        double dx = xValue - x_.at(k);
        return (2.0 * c_.at(k) + 6.0 * dx * d_.at(k));

    }


    template <CurveMode::Mode mode, CurveMode::Interpolation T, size_t N>
    std::ostream& operator<< (std::ostream& output, const Curve<mode, T, N>& c)
    {
        for (unsigned i = 0; i < c.x_.size(); ++i)
            output << c.x_.at(i) << " ";
        output << std::endl;
        for (unsigned i = 0; i < c.y_.size(); ++i)
            output << c.y_.at(i) << " ";
        output << std::endl;
        for (std::vector<double>::const_iterator it = c.b_.begin(); it < c.b_.end(); ++it)
            output << *it << " ";
        output << std::endl;
        for (std::vector<double>::const_iterator it = c.c_.begin(); it < c.c_.end(); ++it)
            output << *it << " ";
        output << std::endl;
        for (std::vector<double>::const_iterator it = c.d_.begin(); it < c.d_.end(); ++it)
            output << *it << " ";
        output << std::endl;
        return output;
    }
}
 
