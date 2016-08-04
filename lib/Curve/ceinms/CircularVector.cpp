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

#include <vector>
using std::vector;
#include <iterator>
#include <iostream>
#include <stdlib.h>

namespace ceinms {
    //const unsigned DEFAULT_MAX_SIZE = 15;

    template<typename T, size_t N>
    CircularVector<T, N>::CircularVector()
        :count_(0), beg_(0), v_({ .0 }) {
        //empty
    }


    template<typename T, size_t N>
    const T& CircularVector<T, N>::at(unsigned i) const {

        if (i >= count_) {
            std::cout << "CircularVector out of range\n";
            exit(EXIT_FAILURE);
        }

        return v_.at((beg_ + i) % N);

    }


    template<typename T, size_t N>
    T& CircularVector<T, N>::at(unsigned i) {

        if (i >= count_) {
            std::cout << "CircularVector out of range\n";
            exit(EXIT_FAILURE);
        }

        return v_.at((beg_ + i) % N);

    }

    template<typename T, size_t N>
    const T& CircularVector<T, N>::operator[](unsigned i) const {

        return v_[(beg_ + i) % N];
    }


    template<typename T, size_t N>
    T& CircularVector<T, N>::operator[](unsigned i) {

        return v_[(beg_ + i) % N];
    }


    template<typename T, size_t N>
    CircularVector<T, N>& CircularVector<T, N>::operator=(const CircularVector<T, N>& rhs) {

        v_ = rhs.v_;
        count_ = rhs.count_;
        beg_ = rhs.beg_;

        return *this;
    }

    //template<typename T, size_t N>
    //CircularVector<T, N>& CircularVector<T, N>::operator=(const std::vector<T>& rhs) {
    //
    //    v_ = rhs;
    //    count_ = rhs.size();
    //    beg_ = 0;
    //
    //    return *this;
    //}


    template<typename T, size_t N>
    CircularVector<T, N>::CircularVector(const CircularVector< T, N >& rhs) {

        v_ = rhs.v_;
        count_ = rhs.count_;
        beg_ = rhs.beg_;

    }


    //template<typename T, size_t N>
    //void CircularVector<T, N>::resizeMax(unsigned i) {
    //
    //    size_ = i;
    //    v_.resize(size_);
    //    beg_ = 0;
    //    count_ = 0;
    //
    //}


    template<typename T, size_t N>
    void CircularVector<T, N>::clear() {

        v_.fill(.0);
        beg_ = 0;
        count_ = 0;
    }


    template<typename T, size_t N>
    void CircularVector<T, N>::push_back(const T& e) {

        unsigned end = (beg_ + count_) % N;
        v_.at(end) = e;
        if (count_ == N)
            beg_ = (beg_ + 1) % N;
        else
            ++count_;
    }


    template<typename T, size_t N>
    void CircularVector<T, N>::pop_back() {

        if (count_ != 0)
            --count_;
    }


    template<typename T, size_t N>
    unsigned CircularVector<T, N>::size() const {

        return count_;
    }


    template<typename T, size_t N>
    bool CircularVector<T, N>::isFull() {

        return count_ >= N;

    }


    template<typename T, size_t N>
    bool CircularVector<T, N>::isEmpty() {

        return (count_ == 0);
    }


    template<typename T, size_t N>
    bool CircularVector<T, N>::empty() const
    {
        return size() == 0;
    };
}
