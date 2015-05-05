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
