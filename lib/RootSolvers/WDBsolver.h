#ifndef WDBsolver_h
#define WDBsolver_h

#include <limits>
#include <math.h>

template <typename T> int sgn(T val) {
    return (T(0) < val) - (val < T(0));
}

template <typename T>
T sign(const T& a, const T& b) {

        return sgn(b)*fabs(a);
}

template <typename T> 
double wdbSolve(T &func, double x1, double x2, double tol) {

    const int ITMAX = 100; 
    const double EPS = std::numeric_limits<double>::epsilon();
    
    double a = x1, b = x2, c = x2, d , e ,fa = func(a),fb = func(b), fc, p, q, r, s, tol1, xm;
    if ((fa > 0.0 && fb > 0.0) || (fa < 0.0 && fb < 0.0))
        throw("Root must be bracketed in wdbSolve");
    fc=fb;
    for (int iter=0; iter<ITMAX; iter++) {
        if ((fb > 0.0 && fc > 0.0) || (fb < 0.0 && fc < 0.0)) {
            c=a; 
            fc=fa;
            e = d = b-a;
        }
        if (fabs(fc) < fabs(fb)) {
            a=b;
            b=c;
            c=a;
            fa=fb;
            fb=fc;
            fc=fa;
        }

        tol1 = 2.0*EPS*fabs(b) + 0.5*tol;
        xm = 0.5*(c-b);
        if (fabs(xm) <= tol1 || fb == 0.0) 
            return b;
        if (fabs(e) >= tol1 && fabs(fa) > fabs(fb)) {
            s=fb/fa;
            if (a == c) {
                p = 2.0*xm*s;
                q = 1.0 - s;
            } else {
                q = fa/fc;
                r = fb/fc;
                p = s*(2.0*xm*q*(q - r) - (b - a)*(r - 1.0));
                q = (q - 1.0)*(r - 1.0)*(s - 1.0);
            }
            if (p > 0.0) 
                q = -q; 
            p = fabs(p);
            double min1 = 3.0*xm*q - fabs(tol1*q);
            double min2 = abs(e*q);
            if (2.0*p < (min1 < min2 ? min1 : min2)) {
                e = d;
                d = p/q;
            } else {
                d = xm;
                e=d;
            }
        } else { 
            d=xm;
            e=d;
        }
        a = b; 
        fa = fb;
        if (fabs(d) > tol1) 
            b += d;
        else
        b += sign(tol1, xm);
        fb = func(b);
    }
    throw("Maximum number of iterations exceeded in zbrent");
}


#endif