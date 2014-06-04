#ifndef Array2Vector_h
#define Array2Vector_h
#include <vector>
#include "OpenSim/Common/Array.h"


class Array2Vector{
public:
    template<typename T>
    static void convert(const OpenSim::Array<T>& srcArray, std::vector<T>& dstVector) {
        dstVector.clear();
        int size = srcArray.getSize();
        dstVector.resize(size);
        for(int i = 0; i < size; ++i)
        dstVector.at(i) = srcArray.get(i);
    }

};

#endif