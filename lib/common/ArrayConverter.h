//__________________________________________________________________________
// Author(s): Claudio Pizzolato - October 2013
// email:  claudio.pizzolato@griffithuni.edu.au
//
// DO NOT REDISTRIBUTE WITHOUT PERMISSION
//__________________________________________________________________________
//

#ifndef ArrayConverter_h
#define ArrayConverter_h

#include <vector>

class ArrayConverter{
public:
    template<typename T>
    static void toStdVector(const OpenSim::Array<T>& srcArray, std::vector<T>& dstVector) {
        dstVector.clear();
        int size = srcArray.getSize();
        dstVector.resize(size);
        for(int i = 0; i < size; ++i)
        dstVector.at(i) = srcArray.get(i);
    }
    template<typename T>
    static void fromStdVector(OpenSim::Array<T>& dstArray, const std::vector<T>& srcVector) {
        for(typename std::vector<T>::const_iterator it(srcVector.begin()); it != srcVector.end(); ++it)
            dstArray.append(*it);
    }


};

#endif