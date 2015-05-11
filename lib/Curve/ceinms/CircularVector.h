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
 * Author(s): C. Pizzolato                                                    *
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

#ifndef ceinms_CircularVector_h
#define ceinms_CircularVector_h
#include <array>
#include <iostream>

namespace ceinms {
    template<typename T, size_t N = 15>
    class CircularVector {

    public:
        using container_type = std::array < T, N > ;
        CircularVector();
        CircularVector(const CircularVector& rhs);
        CircularVector& operator=(const CircularVector& rhs);
        //    CircularVector<T, N>& operator=(const std::vector<T>& rhs);
        const T& operator[](unsigned i) const;
        T& operator[](unsigned i);
        const T& at(unsigned i) const;
        T& at(unsigned i);

        T& back() { return at((count_ - 1)); }
        const T& back() const { return at((count_ - 1)); }

        T& front() { return at(0); }
        const T& front() const { return at(0); }

        // void resizeMax(unsigned i);
        void clear();
        void push_back(const T& e);
        void pop_back();
        unsigned getMaxsize() const { return N; }
        unsigned size() const;
        bool empty() const;
    private:
        bool isFull();
        bool isEmpty();
        container_type v_;
        unsigned beg_, count_;

    };

    template<typename T, size_t N>
    std::ostream& operator<< (std::ostream & out, const  CircularVector<T, N>& rhs) {

        out << "{";
        if (!rhs.empty()) {
            for (size_t i(0); i < rhs.size() - 1; ++i)
                out << rhs[i] << ", ";
            out << rhs.back();
        }
        out << "}";
        return out;
    }
}

#include "CircularVector.cpp"
#endif
