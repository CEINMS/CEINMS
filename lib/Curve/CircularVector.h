//__________________________________________________________________________
// Author(s): Claudio Pizzolato - September 2013
// email:  claudio.pizzolato@griffithuni.edu.au
//
// DO NOT REDISTRIBUTE WITHOUT PERMISSION
//__________________________________________________________________________
// 

#ifndef CircularVector_h
#define CircularVector_h
#include <vector>

template<typename T>
class CircularVector {
    
public:
    CircularVector();
    CircularVector(const CircularVector<T>& rhs); 
    CircularVector<T>& operator=(const CircularVector<T>& rhs);
    CircularVector<T>& operator=(const std::vector<T>& rhs);
    T operator[](unsigned i) const;
    T at(unsigned i) const;
    T back() { return at((count_-1)); }
    void resizeMax(unsigned i);
    void clear();
    void push_back(const T& e);
    void pop_back();
    unsigned getMaxsize() { return size_; }
    unsigned size() const;
private:
    void rotateR();
    void rotateL();
    bool isFull();
    bool isEmpty();
    std::vector<T> v_;
    unsigned beg_, count_, size_;
    
};

#include "CircularVector.cpp"

#endif 
