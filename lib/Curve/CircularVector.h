//__________________________________________________________________________
// Author(s): Claudio Pizzolato - September 2013
// email:  claudio.pizzolato@griffithuni.edu.au
//
// DO NOT REDISTRIBUTE WITHOUT PERMISSION
//__________________________________________________________________________
// 

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
